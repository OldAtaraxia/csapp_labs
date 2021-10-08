# Shell lab

tsh.c是一个shell程序的骨架, 需要补全一些空函数

- eval: 解析然后执行命令
	- 在`eval`里父进程fork子进程之前需要用`sigprocmask`来阻塞`SIGCHLD`, 之后再解除阻塞.
	- 子进程会继承父进程的阻塞位向量所以子进程要unblock SIGCHLD
- builtin_cmd: 识别并执行内置命令: quit, fg, bg, jobs
- do_bgfg: 实现内置命令bg和fg
- Waitfg: 等待一个前台任务完成(?)
- sigchld_handler: 捕获SIGCHLD信号
- sigint_handler: 捕获SIGINT(ctrl+c)信号
- sigtstp_handler: 捕获SIGSTP(ctrl+z)信号

要求tsh的features:

- prompt应该是"tsh> "
- 命令由name和arguments组成, 然后name要么是内置的命令,这时候tsh需要立即处理它然后等待下一个命令, 否则将其看做可执行文件的路径, 开一个子进程然后在其中执行这个文件(这种情况下把那个子进程看做job)
- 以`&`结尾的命令在后台运行, 否则在前台运行.
- 每个job有自己的pid和job id(jid), jid应该记作`%`+ 整数, pid应该是单独的整数
- tsh应该支持的内置指令:
	- `quit`: 停止shell
	- `jobs`: 列出所有的后台进程
	- `bg <job>`: 通过发送给<job>进程`SIGCONT`来重启进程, 然后在后台运行它, <job>可以是pid也可以是jid
	- `fg <job>`: 通过发送给<job>进程`SIGCONT`来重启进程, 然后在前台运行它, <job>可以是pid也可以是jid
- tsh需要把它的僵尸子进程都回收掉, 如果有job终止了但是其信号没有被收到, ths应该能意识到然后打印一条信息, 进程的pid+描述

csapp第八章有很多代码示例,仿照那些实例来写就好了...
* eval
```C
void eval(char *cmdline) 
{
    char** argv = malloc(sizeof(char*) * MAXARGS); /* argv array */
    char* buf = malloc(sizeof(char) * MAXLINE);
    int bg, pid; 
    sigset_t mask, prev, mask_all;

    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigfillset(&mask_all);

    strcpy(buf, cmdline);
    bg = parseline(buf, argv);
    if(argv[0] == NULL) return; /* 忽略空行 */
    if(!builtin_cmd(argv)) {
        sigprocmask(SIG_BLOCK, &mask, &prev);
        if((pid = fork()) == 0) {
            /* 子进程 */
            setpgid(0, 0);
            sigprocmask(SIG_SETMASK, &prev, NULL);
            if(execve(argv[0], argv, NULL) < 0) {
                printf("%s: Command not found\n", argv[0]);
                exit(0);
            }
        }
        /* 父进程 */
        sigprocmask(SIG_BLOCK, &mask_all, NULL);
        addjob(jobs, pid, bg ? BG : FG, cmdline);
        sigprocmask(SIG_SETMASK, &prev, NULL);
        if(!bg) {
            waitfg(pid);
        }else {
            printf("[%d] (%d) %s", pid2jid(pid), pid, cmdline);
        }
    }
    return;
}
```
* builtin_cmd
```C
int builtin_cmd(char **argv) 
{
    if(!strcmp(argv[0], "quit")) {
        exit(0);
    }else if(!strcmp(argv[0], "bg")) {
        do_bgfg(argv);
        return 1;
    }else if(!strcmp(argv[0], "fg")) {
        do_bgfg(argv);
        return 1;
    }else if(!strcmp(argv[0], "jobs")) {
        listjobs(jobs);
        return 1;
    }
    return 0;     /* not a builtin command */
}
```
* waitfg
```C
void waitfg(pid_t pid)
{
    while(pid == fgpid(jobs)) {
        sleep(0.5);
    }
    if(verbose) printf("waitfg: Process (%d) no longer the fg process\n", pid);
    return;
}
```
* do_bgfg
```C
void do_bgfg(char **argv) 
{
    int pid = -1, jid = -1;
    sigset_t mask_all, prev;
    struct job_t* job_ptr;

    sigfillset(&mask_all);
    if(argv[1] == NULL) {
        printf("%s command requires PID or %%jobid argument\n", argv[0]);
        return;
    }
    if(argv[1][0] == '%') {
        jid = 0;
        char* ctr = argv[1];
        ctr++;
        while((*ctr) != '\0') {
            jid *= 10;
            jid += (*ctr) - '0';
            ctr++;
        }
    }else if(argv[1][0] >= '0' && argv[1][0] <= '9'){
        pid = 0;
        char* ctr = argv[1];
        while((*ctr) != '\0') {
            pid *= 10;
            pid += (*ctr) - '0';
            ctr++;
        }
    }else {
        printf("%s: argument must be a PID or %%jobid\n", argv[0]);
        return;
    }
    if(pid == -1) { 
        job_ptr = getjobjid(jobs, jid);
        if(job_ptr == NULL) {
            printf("%%%d: No such job\n", jid);
            return;
        }
        pid = job_ptr->pid;
    }else {
        job_ptr = getjobpid(jobs, pid);
        if(job_ptr == NULL) {
            printf("(%d): No such process\n", pid);
            return;
        }
    }
    if(!strcmp(argv[0], "bg")) {
        sigprocmask(SIG_BLOCK, &mask_all, &prev);
        job_ptr->state = BG;
        sigprocmask(SIG_SETMASK, &prev, NULL);
        kill(-pid, 18);
        printf("[%d] (%d) %s", pid2jid(pid), pid, job_ptr->cmdline);
    }else {
        sigprocmask(SIG_BLOCK, &mask_all, &prev);
        job_ptr->state = FG;
        sigprocmask(SIG_SETMASK, &prev, NULL);
        kill(-pid, 18);
        waitfg(pid);
    }
    return;
}
```

* sigchld_handler
```C
void sigchld_handler(int sig) 
{
    int olderrno = errno, jid;
    int status; // 记录waitpid的状态
    sigset_t mask_all, prev_all;
    pid_t pid;

    if(verbose) printf("sigchld_handler: entering\n");
    sigfillset(&mask_all);
    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
        jid = pid2jid(pid);
        if(WIFEXITED(status)) {
            if(verbose) printf("sigchld_handler: Job [%d] (%d) deleted\n", jid, pid);            
            sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
            deletejob(jobs, pid);
            sigprocmask(SIG_SETMASK, &prev_all, NULL);
            if(verbose) printf("Job [%d] (%d) terminates OK (status 0)\n", jid, pid);
        }else if(WIFSTOPPED(status)) {
            printf("Job [%d] (%d) stopped by signal %d\n", jid, pid, WSTOPSIG(status));
            sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
            getjobpid(jobs, pid) -> state = ST;
            sigprocmask(SIG_SETMASK, &prev_all, NULL);
        }else if(WIFSIGNALED(status)) {
            printf("Job [%d] (%d) termanated by signal 2\n", jid, pid);
            sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
            deletejob(jobs, pid);
            sigprocmask(SIG_SETMASK, &prev_all, NULL);
        }
    }
    errno = olderrno;
    if(verbose) printf("sigchld_handler: exiting\n");
    return;
}
```
* sigint_handler
```C
void sigint_handler(int sig) 
{
    pid_t pid;
    pid = fgpid(jobs);
    // printf("\n%d %d\n", pid, getpid());
    if(pid != 0) {
        kill(-pid, 2);
    }
    // printf("over\n");
    return;
}
```
* sigstp_handler
```C
void sigtstp_handler(int sig) 
{
    int pid;
    pid = fgpid(jobs);
    // printf("\n%d %d\n", pid, getpid());
    if(pid != 0) {
        kill(-pid, 20);
    }
    // printf("over\n");
    return;
}
```