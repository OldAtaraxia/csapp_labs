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

## Checking your work

`tshref`是reference solution

通过`sdriver.pl`与提供的trace文件进行测试

```
./sdriver.pl -t trace01.txt -s ./tsh -a "-p"
```

或者

```
make test01
```

与reference solution的结果作比较的话

```
./sdriver.pl -t trace01.txt -s ./tshref -a "-p"
```

或者

```
make rtest01
```

`tshref.out`里有所有样例的答案, 

### Hint

![img](https://mf3qv90vh2.feishu.cn/space/api/box/stream/download/asynccode/?code=M2RjZWYxNWIyZWY5MjliNDNhNDI4ZGY3YjMzY2M0ZDBfMlJLQkJOTUNkM0xySTRSb3R0Zm1lbWNkUTYwZjc5SnBfVG9rZW46Ym94Y24xdkI0TmhBc1dhZU15MDQwZ1BtdkZlXzE2MzMzNTQ2MjE6MTYzMzM1ODIyMV9WNA)