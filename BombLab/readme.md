bomb lab

指导文档 http://csapp.cs.cmu.edu/3e/bomblab.pdf

首先看下bomb.c, 其流程是这样的

```
    initialize_bomb();

    printf("Welcome to my fiendish little bomb. You have 6 phases with\n");
    printf("which to blow yourself up. Have a nice day!\n");

    /* Hmm...  Six phases must be more secure than one phase! */
    input = read_line();             /* Get input                   */
    phase_1(input);                  /* Run the phase               */
    phase_defused();                 /* Drat!  They figured it out!
                                      * Let me know how they did it. */
    printf("Phase 1 defused. How about the next one?\n");

    /* The second phase is harder.  No one will ever figure out
     * how to defuse this... */
    input = read_line();
    phase_2(input);
    phase_defused();
    printf("That's number 2.  Keep going!\n");

    /* I guess this is too easy so far.  Some more complex code will
     * confuse people. */
    input = read_line();
    phase_3(input);
    phase_defused();
    printf("Halfway there!\n");

    /* Oh yeah?  Well, how good is your math?  Try on this saucy problem! */
    input = read_line();
    phase_4(input);
    phase_defused();
    printf("So you got that one.  Try this one.\n");
    
    /* Round and 'round in memory we go, where we stop, the bomb blows! */
    input = read_line();
    phase_5(input);
    phase_defused();
    printf("Good work!  On to the next...\n");

    /* This phase will never be used, since no one will get past the
     * earlier ones.  But just in case, make this one extra hard. */
    input = read_line();
    phase_6(input);
    phase_defused();

    /* Wow, they got it!  But isn't something... missing?  Perhaps
     * something they overlooked?  Mua ha ha ha ha! */
```

也就是说每一关都是调用`phase_x`来处理输入的.

开始读汇编

### phase_1

即把读入的字符串作为参数传递给phase_1.

phase_1

```
0000000000400ee0 <phase_1>:
  400ee0:        48 83 ec 08                  sub    $0x8,%rsp
  400ee4:        be 00 24 40 00               mov    $0x402400,%esi
  400ee9:        e8 4a 04 00 00               callq  401338 <strings_not_equal>
  400eee:        85 c0                        test   %eax,%eax
  400ef0:        74 05                        je     400ef7 <phase_1+0x17>
  400ef2:        e8 43 05 00 00               callq  40143a <explode_bomb>
  400ef7:        48 83 c4 08                  add    $0x8,%rsp
  400efb:        c3                           retq   
```

可以看到phase1把一个立即数$0x402400移入了%esi(rsi), 显然是作为参数的.之后去调用了strings_not_equal函数, 接着检查了eax的值, 若为0则条件跳转到最后,否则引爆炸弹...

看一下$0x402400的内容:

```
(gdb) x/s 0x402400
0x402400:       "Border relations with Canada have never been better."
```

输入试一试

```
(gdb) run
Starting program: /home/oldataraxia/labs/csapp/BombLab/bomb
Welcome to my fiendish little bomb. You have 6 phases with
which to blow yourself up. Have a nice day!
Border relations with Canada have never been better.

Breakpoint 1, 0x0000000000400ee0 in phase_1 ()
(gdb) continue
Continuing.
Phase 1 defused. How about the next one?
```

题外话:

![img](https://mf3qv90vh2.feishu.cn/space/api/box/stream/download/asynccode/?code=NDYyNmQ4MzAyNzlmZmNkNWE2MTQxOTBkMjM4M2U3NjFfMzhBdkNXZXlwMDZoS1lhaWxUdmlqVEY3TmRuZWh2YkVfVG9rZW46Ym94Y251bGszMjlkUDF1WDBoaXM2T0lWRXgwXzE2MzIyOTUyMDU6MTYzMjI5ODgwNV9WNA)

read_lines的返回值是读入的字符串在内存中存放的位置。

### phase_2

phase_2的调用过程

```
  400e4e:        e8 4b 06 00 00               callq  40149e <read_line>
  400e53:        48 89 c7                     mov    %rax,%rdi
  400e56:        e8 a1 00 00 00               callq  400efc <phase_2>
  400e5b:        e8 64 07 00 00               callq  4015c4 <phase_defused>
```

与phase_1一样

phase_2具体的过程

```
0000000000400efc <phase_2>:
  400efc:        55                           push   %rbp
  400efd:        53                           push   %rbx
  400efe:        48 83 ec 28                  sub    $0x28,%rsp
  400f02:        48 89 e6                     mov    %rsp,%rsi
  400f05:        e8 52 05 00 00               callq  40145c <read_six_numbers>
  400f0a:        83 3c 24 01                  cmpl   $0x1,(%rsp)
  400f0e:        74 20                        je     400f30 <phase_2+0x34>
  400f10:        e8 25 05 00 00               callq  40143a <explode_bomb>
  400f15:        eb 19                        jmp    400f30 <phase_2+0x34>
  400f17:        8b 43 fc                     mov    -0x4(%rbx),%eax // <phase_2+0x1b>
  400f1a:        01 c0                        add    %eax,%eax
  400f1c:        39 03                        cmp    %eax,(%rbx)
  400f1e:        74 05                        je     400f25 <phase_2+0x29>
  400f20:        e8 15 05 00 00               callq  40143a <explode_bomb>
  400f25:        48 83 c3 04                  add    $0x4,%rbx // <phase_2+0x29>
  400f29:        48 39 eb                     cmp    %rbp,%rbx
  400f2c:        75 e9                        jne    400f17 <phase_2+0x1b>
  400f2e:        eb 0c                        jmp    400f3c <phase_2+0x40>
  400f30:        48 8d 5c 24 04               lea    0x4(%rsp),%rbx // <phase_2+0x34>
  400f35:        48 8d 6c 24 18               lea    0x18(%rsp),%rbp
  400f3a:        eb db                        jmp    400f17 <phase_2+0x1b>
  400f3c:        48 83 c4 28                  add    $0x28,%rsp // <phase_2+0x40>
  400f40:        5b                           pop    %rbx
  400f41:        5d                           pop    %rbp
  400f42:        c3                           retq   
```

发现它调用了一个函数`read_six_numbers`

```
000000000040145c <read_six_numbers>:
  40145c:        48 83 ec 18                  sub    $0x18,%rsp
  401460:        48 89 f2                     mov    %rsi,%rdx
  401463:        48 8d 4e 04                  lea    0x4(%rsi),%rcx
  401467:        48 8d 46 14                  lea    0x14(%rsi),%rax
  40146b:        48 89 44 24 08               mov    %rax,0x8(%rsp)
  401470:        48 8d 46 10                  lea    0x10(%rsi),%rax
  401474:        48 89 04 24                  mov    %rax,(%rsp)
  401478:        4c 8d 4e 0c                  lea    0xc(%rsi),%r9
  40147c:        4c 8d 46 08                  lea    0x8(%rsi),%r8
  401480:        be c3 25 40 00               mov    $0x4025c3,%esi
  401485:        b8 00 00 00 00               mov    $0x0,%eax
  40148a:        e8 61 f7 ff ff               callq  400bf0 <__isoc99_sscanf@plt>
  40148f:        83 f8 05                     cmp    $0x5,%eax
  401492:        7f 05                        jg     401499 <read_six_numbers+0x3d>
  401494:        e8 a1 ff ff ff               callq  40143a <explode_bomb>
  401499:        48 83 c4 18                  add    $0x18,%rsp
  40149d:        c3                           retq   
```

- 把rsi的值移入rdx

- 把rsi+0x4移入rcx

- 把rsi+0x14移入rax

- 把rax值移入memory[rsp+0x8]

- 把rsi+0x10移入rax

- 把rax的值移入memory[rsp]

- 把rsi+0xc移入r9

- 把rsi+0x8移入r8

- 令rsi等于0x4025c3, rax等于0x0

- 调用sscanf

- 比较rax与0x5的大小,相等就跳到40145c+3d = 401499

众所周知C 库函数 **int sscanf(const char \*str, const char \*format, ...)** 从字符串读取格式化输入。如果成功，该函数返回成功匹配和赋值的个数。如果到达文件末尾或发生读错误，则返回 EOF。这里如果读入的数字大于5个就跳转到read_six_number+0x3d, 否则就爆炸(是的,验证过了)

这里其实是读入了六个整数然后放在了phase_2中分配的栈上的空间里, 比如我输入1, 2, 3, 4,5,6

```
(gdb) x $rsp
0x7ffffffee260: 0x00000001
(gdb) x $rsp+0x4
0x7ffffffee264: 0x00000002
(gdb) x $rsp+0x8
0x7ffffffee268: 0x00000003
(gdb) x $rsp+0xc
0x7ffffffee26c: 0x00000004
(gdb) x $rsp+0x10
0x7ffffffee270: 0x00000005
(gdb) x $rsp+0x14
0x7ffffffee274: 0x00000006
```

重大发现啊.

那么我们继续看, 1与%rsp(即输入的第一个数字)比较,不相等则引爆,相等则跳到400f30处, 向rbx中写入rsp+0x4, 向rbp中写入rsp+0x18, 然后跳到400f17处.然后向rax中写入memory[rsp],让这个值加上自身(*2), 然后与rsp+0x4的值比较,看是否是两倍, 不是就引爆炸弹,是的话后跳到400F25, 让rbx中地址+0x4(),然后将rbp与rbx中的值作比较, 若不等于则跳转到400f17重复之前的操作.

所以需要的序列是`1 2 4 8 16 32`

```
Welcome to my fiendish little bomb. You have 6 phases withwhich to blow yourself up. Have a nice day!Border relations with Canada have never been better.Phase 1 defused. How about the next one?1 2 4 8 16 32Breakpoint 1, 0x000000000040145c in read_six_numbers ()(gdb) continueContinuing.That's number 2.  Keep going!
```

### phase_3

调用过程与前两个是同样的

```
  400e60: bf ed 22 40 00        mov    $0x4022ed,%edi
  400e65: e8 a6 fc ff ff        callq  400b10 <puts@plt>
  400e6a: e8 2f 06 00 00        callq  40149e <read_line>
  400e6f: 48 89 c7              mov    %rax,%rdi
  400e72: e8 cc 00 00 00        callq  400f43 <phase_3>
  400e77: e8 48 07 00 00        callq  4015c4 <phase_defused>
0000000000400f43 <phase_3>:
  400f43: 48 83 ec 18           sub    $0x18,%rsp
  400f47: 48 8d 4c 24 0c        lea    0xc(%rsp),%rcx
  400f4c: 48 8d 54 24 08        lea    0x8(%rsp),%rdx
  400f51: be cf 25 40 00        mov    $0x4025cf,%esi
  400f56: b8 00 00 00 00        mov    $0x0,%eax
  400f5b: e8 90 fc ff ff        callq  400bf0 <__isoc99_sscanf@plt>
  400f60: 83 f8 01              cmp    $0x1,%eax
  400f63: 7f 05                 jg     400f6a <phase_3+0x27>
  400f65: e8 d0 04 00 00        callq  40143a <explode_bomb>
  400f6a: 83 7c 24 08 07        cmpl   $0x7,0x8(%rsp)
  400f6f: 77 3c                 ja     400fad <phase_3+0x6a>
  400f71: 8b 44 24 08           mov    0x8(%rsp),%eax
  400f75: ff 24 c5 70 24 40 00  jmpq   *0x402470(,%rax,8) // confusing
  400f7c: b8 cf 00 00 00        mov    $0xcf,%eax
  400f81: eb 3b                 jmp    400fbe <phase_3+0x7b>
  400f83: b8 c3 02 00 00        mov    $0x2c3,%eax
  400f88: eb 34                 jmp    400fbe <phase_3+0x7b>
  400f8a: b8 00 01 00 00        mov    $0x100,%eax
  400f8f: eb 2d                 jmp    400fbe <phase_3+0x7b>
  400f91: b8 85 01 00 00        mov    $0x185,%eax
  400f96: eb 26                 jmp    400fbe <phase_3+0x7b>
  400f98: b8 ce 00 00 00        mov    $0xce,%eax
  400f9d: eb 1f                 jmp    400fbe <phase_3+0x7b>
  400f9f: b8 aa 02 00 00        mov    $0x2aa,%eax
  400fa4: eb 18                 jmp    400fbe <phase_3+0x7b>
  400fa6: b8 47 01 00 00        mov    $0x147,%eax
  400fab: eb 11                 jmp    400fbe <phase_3+0x7b>
  400fad: e8 88 04 00 00        callq  40143a <explode_bomb>
  400fb2: b8 00 00 00 00        mov    $0x0,%eax
  400fb7: eb 05                 jmp    400fbe <phase_3+0x7b>
  400fb9: b8 37 01 00 00        mov    $0x137,%eax
  400fbe: 3b 44 24 0c           cmp    0xc(%rsp),%eax // koko
  400fc2: 74 05                 je     400fc9 <phase_3+0x86>
  400fc4: e8 71 04 00 00        callq  40143a <explode_bomb>
  400fc9: 48 83 c4 18           add    $0x18,%rsp
  400fcd: c3                    retq   
```

可以看到`sscanf`, 格式为`%d %d`

![img](https://mf3qv90vh2.feishu.cn/space/api/box/stream/download/asynccode/?code=M2MyYTAyZTgxM2FlNTZjOGNkYTQ3OTUzMmRjNjkzOThfSWlBMTVLMHl3aXd5d01IdHpMZ3J6bTUzcGxKeDNEbTJfVG9rZW46Ym94Y25pVnM1MU1DTEliOTZ3V0VFMkZLOWxiXzE2MzIyOTUyMDU6MTYzMjI5ODgwNV9WNA)

- 栈上开0x18的空间

- rcx里写rsp+0xc

- rdx里写rsp+0x8

- rsi里写立即数$0x4025cf(翻译成字符串是"%d %d")

![img](https://mf3qv90vh2.feishu.cn/space/api/box/stream/download/asynccode/?code=MzZhODk2MGZkMWE5MGM5NjA3YWM3NjEwNDFmNjlhZjRfQVpQWlptdW9rbVdOeTNvM0NLbXl0aHltcURtaEFBYmNfVG9rZW46Ym94Y25DT1FwYlZUYThaUkFYWHRsWXZlUG1kXzE2MzIyOTUyMDU6MTYzMjI5ODgwNV9WNA)

- rax里写立即数0x0

- 调用sscanf拿到两个%d, 这里可以看出来rdx和rcx是作为sscanf的参数的,读入的int被存在rsp+0x8和rsp+0xc处了.

- 比较读到的值的个数和1,若大于则跳到400f6a, 否则爆炸

- (400f6a)比较memory[rsp+0x8](也就是读入的第一个数)与立即数0x7,大于则爆炸(跳到400fad, 那里是爆炸函数)

- rax中写入memory[rsp+0x8](也就是读入的第一个数)

- 跳到memory[0x402470 + 8 * rax]中写的内存

- 到这里因为不知道跳转到哪里去了所以已经没法判断执行顺序了,但是注意到从这里开始是一大堆意义不明的`mov`和`jmp`的交替出现

- 把各种奇怪的立即数塞入eax然后jmp去400fbe, 这提示我刚才的跳转的目的地应该就是这里的这一大堆mov.

- 再去看看400fbe, 比较rax和memory[0xc+rsp], 也就是memory[读入的第二个数], 若相等则函数返回,否则爆炸.

- 这时候可以回去看上面的意义不明的跳转了, 我们去memory[0x402470]附近看看

```
(gdb) x/a 0x402470
0x402470:       0x400f7c <phase_3+57>
(gdb) x/a 0x402470 + 0x8
0x402478:       0x400fb9 <phase_3+118>
(gdb) x/a 0x402470 + 0x8 + 0x8
0x402480:       0x400f83 <phase_3+64>
(gdb) x/a 0x402470 + 0x8 + 0x8 + 0x8
0x402488:       0x400f8a <phase_3+71>
(gdb) x/a 0x402470 + 0x8 + 0x8 + 0x8 + 0x8
0x402490:       0x400f91 <phase_3+78>
(gdb) x/a 0x402470 + 0x8 + 0x8 + 0x8 + 0x8 + 0x8
0x402498:       0x400f98 <phase_3+85>
(gdb) x/a 0x402470 + 0x8 + 0x8 + 0x8 + 0x8 + 0x8 + 0x8
0x4024a0:       0x400f9f <phase_3+92>
(gdb) x/a 0x402470 + 0x8 + 0x8 + 0x8 + 0x8 + 0x8 + 0x8 + 0x8
0x4024a8:       0x400fa6 <phase_3+99>
```

果然有很大的收获, 这些地址就是上面mov的地址. 根据读到的第一个数的不同我会带着不同的rax值到达400fbe.而400fbe这里是拿rax和读入的第二个数比较,也就是说只要两个数是对应的,炸弹就不会爆炸.

那么对应的答案有:(0,207), (1, 311), (2,707),(3,256),(4,389),(5,206),(6,682),(7,327)

![img](https://mf3qv90vh2.feishu.cn/space/api/box/stream/download/asynccode/?code=ODJmY2RkMTcyN2FhNzA0Yzk0Y2MwOGUyOWQ1MzYxMTVfdEt3VWl6eEZpTnNObkhFOTZPMW1NWGkzTmg4RE94S0tfVG9rZW46Ym94Y25YRXFtYUNsTHlVNGVTNUFBVTIzREVkXzE2MzIyOTUyMDU6MTYzMjI5ODgwNV9WNA)

经过测试看到所有的组都满足条件.

btw笑死我了2333

![img](https://mf3qv90vh2.feishu.cn/space/api/box/stream/download/asynccode/?code=OTEzMWJjZTFiMTAzNjhkMDczZTJiZDgxMDk1YTRjNjVfbmhXdERkNk8yVFNIaXV2MGh5d0NUVXVIOWtEb3Zrd1lfVG9rZW46Ym94Y245R0dBSnNiS2NjRHp4aEpPSkcyOExiXzE2MzIyOTUyMDU6MTYzMjI5ODgwNV9WNA)

### phase_4

```
000000000040100c <phase_4>:
  40100c: 48 83 ec 18           sub    $0x18,%rsp
  401010: 48 8d 4c 24 0c        lea    0xc(%rsp),%rcx
  401015: 48 8d 54 24 08        lea    0x8(%rsp),%rdx
  40101a: be cf 25 40 00        mov    $0x4025cf,%esi
  40101f: b8 00 00 00 00        mov    $0x0,%eax
  401024: e8 c7 fb ff ff        callq  400bf0 <__isoc99_sscanf@plt>
  401029: 83 f8 02              cmp    $0x2,%eax
  40102c: 75 07                 jne    401035 <phase_4+0x29>
  40102e: 83 7c 24 08 0e        cmpl   $0xe,0x8(%rsp)
  401033: 76 05                 jbe    40103a <phase_4+0x2e>
  401035: e8 00 04 00 00        callq  40143a <explode_bomb>
  40103a: ba 0e 00 00 00        mov    $0xe,%edx
  40103f: be 00 00 00 00        mov    $0x0,%esi
  401044: 8b 7c 24 08           mov    0x8(%rsp),%edi
  401048: e8 81 ff ff ff        callq  400fce <func4>
  40104d: 85 c0                 test   %eax,%eax
  40104f: 75 07                 jne    401058 <phase_4+0x4c>
  401051: 83 7c 24 0c 00        cmpl   $0x0,0xc(%rsp)
  401056: 74 05                 je     40105d <phase_4+0x51>
  401058: e8 dd 03 00 00        callq  40143a <explode_bomb>
  40105d: 48 83 c4 18           add    $0x18,%rsp
  401061: c3                    retq   
```

前面的部分跟phase_3几乎一样, 用sscanf读两个%d,没读到两个就直接爆炸, 把读到的第一个数与0x额比较,若大于等于就爆炸,大于就继续. 给rdx写入0xe, rsi写入0x0, rdi写入读入的第一个数(memory[rsp+0x8]), 执行一个意义不明的函数func4. 从func4返回后计算rax&rax,如果不是0就爆炸, 然后比较读入的第二个数与0, 如果相等就让返回,否则爆炸.

至此我们可以判断出第二个数是0,第一个数需要让func4返回0.

那现在来看一下func4

```
0000000000400fce <func4>:
  400fce: 48 83 ec 08           sub    $0x8,%rsp
  400fd2: 89 d0                 mov    %edx,%eax
  400fd4: 29 f0                 sub    %esi,%eax
  400fd6: 89 c1                 mov    %eax,%ecx
  400fd8: c1 e9 1f              shr    $0x1f,%ecx
  400fdb: 01 c8                 add    %ecx,%eax
  400fdd: d1 f8                 sar    %eax
  400fdf: 8d 0c 30              lea    (%rax,%rsi,1),%ecx
  400fe2: 39 f9                 cmp    %edi,%ecx
  400fe4: 7e 0c                 jle    400ff2 <func4+0x24>
  400fe6: 8d 51 ff              lea    -0x1(%rcx),%edx
  400fe9: e8 e0 ff ff ff        callq  400fce <func4>
  400fee: 01 c0                 add    %eax,%eax
  400ff0: eb 15                 jmp    401007 <func4+0x39>
  400ff2: b8 00 00 00 00        mov    $0x0,%eax
  400ff7: 39 f9                 cmp    %edi,%ecx
  400ff9: 7d 0c                 jge    401007 <func4+0x39>
  400ffb: 8d 71 01              lea    0x1(%rcx),%esi
  400ffe: e8 cb ff ff ff        callq  400fce <func4>
  401003: 8d 44 00 01           lea    0x1(%rax,%rax,1),%eax
  401007: 48 83 c4 08           add    $0x8,%rsp
  40100b: c3                    retq   
```

rax写入rdx的值(0xe),减去rsi的值(0x0),把rax值移入rcx, rcx的值右移0x1f位(?那不就是0吗), 然后加回rax上, rax再右移(7), rcx写入(rax+rsi)(7), 比较rdi(读入的第一个数)和rcx(7)的值,rcx<=rdi就跳到400ff2, 否则就让rdx = rcx-1,然后递归调用func4,回来之后rax*=2,跳到4001007(函数返回的地方).

从400ff2开始看, 给rax写入0, 比较rcx与rdi, rcx>=rdi则跳到401007,否则rsi = rcx+1, 递归调用func4, 回来后rax = 2 * rax + 1, 然后返回函数.

所以rdi等于7就能完全满足条件

```
Welcome to my fiendish little bomb. You have 6 phases with
which to blow yourself up. Have a nice day!
Border relations with Canada have never been better.
Phase 1 defused. How about the next one?
1 2 4 8 16 32
That's number 2.  Keep going!
1 311
Halfway there!
7 0
So you got that one.  Try this one.
```

### phase_5

```
0000000000401062 <phase_5>:
  401062: 53                    push   %rbx
  401063: 48 83 ec 20           sub    $0x20,%rsp
  401067: 48 89 fb              mov    %rdi,%rbx
  40106a: 64 48 8b 04 25 28 00  mov    %fs:0x28,%rax
  401071: 00 00 
  401073: 48 89 44 24 18        mov    %rax,0x18(%rsp)
  401078: 31 c0                 xor    %eax,%eax
  40107a: e8 9c 02 00 00        callq  40131b <string_length>
  40107f: 83 f8 06              cmp    $0x6,%eax
  401082: 74 4e                 je     4010d2 <phase_5+0x70>
  401084: e8 b1 03 00 00        callq  40143a <explode_bomb>
  401089: eb 47                 jmp    4010d2 <phase_5+0x70>
  40108b: 0f b6 0c 03           movzbl (%rbx,%rax,1),%ecx
  40108f: 88 0c 24              mov    %cl,(%rsp)
  401092: 48 8b 14 24           mov    (%rsp),%rdx
  401096: 83 e2 0f              and    $0xf,%edx
  401099: 0f b6 92 b0 24 40 00  movzbl 0x4024b0(%rdx),%edx
  4010a0: 88 54 04 10           mov    %dl,0x10(%rsp,%rax,1)
  4010a4: 48 83 c0 01           add    $0x1,%rax
  4010a8: 48 83 f8 06           cmp    $0x6,%rax
  4010ac: 75 dd                 jne    40108b <phase_5+0x29>
  4010ae: c6 44 24 16 00        movb   $0x0,0x16(%rsp)
  4010b3: be 5e 24 40 00        mov    $0x40245e,%esi
  4010b8: 48 8d 7c 24 10        lea    0x10(%rsp),%rdi
  4010bd: e8 76 02 00 00        callq  401338 <strings_not_equal>
  4010c2: 85 c0                 test   %eax,%eax
  4010c4: 74 13                 je     4010d9 <phase_5+0x77>
  4010c6: e8 6f 03 00 00        callq  40143a <explode_bomb>
  4010cb: 0f 1f 44 00 00        nopl   0x0(%rax,%rax,1)
  4010d0: eb 07                 jmp    4010d9 <phase_5+0x77>
  4010d2: b8 00 00 00 00        mov    $0x0,%eax //koko
  4010d7: eb b2                 jmp    40108b <phase_5+0x29>
  4010d9: 48 8b 44 24 18        mov    0x18(%rsp),%rax
  4010de: 64 48 33 04 25 28 00  xor    %fs:0x28,%rax
  4010e5: 00 00 
  4010e7: 74 05                 je     4010ee <phase_5+0x8c>
  4010e9: e8 42 fa ff ff        callq  400b30 <__stack_chk_fail@plt>
  4010ee: 48 83 c4 20           add    $0x20,%rsp
  4010f2: 5b                    pop    %rbx
  4010f3: c3                    retq  
```

这里其实首先应该看看`rdi`里面是啥, 是输入的字符串保存的地址......

![img](https://mf3qv90vh2.feishu.cn/space/api/box/stream/download/asynccode/?code=NGY4YzMwYzU1MTIwOTU2MWJiMWFiNmYyZjA2NzdkNDBfazY2STlLcDJ1ZUhUeVU4N1p4cm9VUjRZWWF4aDNTekpfVG9rZW46Ym94Y250SW9zU3RsSUViSVRiWnJWWUcwTWg2XzE2MzIyOTUyMDU6MTYzMjI5ODgwNV9WNA)

%fs:0x28是啥呀......给rbx写入rdi,rax写入%fs:0x28, 给0x18(rsp)写入rax,然后rax做异或......,之后调用string_length,若返回值不等于6就爆炸,等于就跳到4010d2. 

在4010d2让rax等于0, 跳到40108b,

在40108b让ecx等于memory[rbx+rax](即字符串的前四个字符)

让memory[rsp] = rcx的最低8位(第一个字符), 让rdx = memory[rsp], 然后edx与0xf作与操作(只剩下低四位了),然后让edx=memory[0x4024b0+rdx], 这里去看一下0x4042b0里面有什么好东西:

```
(gdb) x/s 0x4024b00x4024b0 <array.3449>:  "maduiersnfotvbylSo you think you can stop the bomb with ctrl-c, do you?"
```

然后把rdx的最低8位()写入memory[0x10+rsp+rax], 然后rax+0x1, 比较rax与0x6,如果不相等就跳回到40108b,重复之前的操作.

在上述循环中rax会从0加到6, 然后会依次写入memory[0x10+rsp]、memory[0x11+rsp]、......、memory[0x15+rsp].

当rax == 0x6时, 让memory[0x16+rsp] = 0x0, 让rsi = 0x40245e, 让rdi等于rsp+0x10(**众所周知rsi和rdi是用作参数传递的**), 然后调用了strings_not_equal函数. 之后rax = rax&rax, 不等于0就爆炸, 等于0就跳到4010d9, 在那里rax=memory[0x18+rsp], rax与fs:0x28(怎么又是你)作异或, 若等于0(众所周知a&a = 0, 一个数异或它本身等于0)则跳到4010ee(函数在那里返回), 否则执行stack_chk_failgrbx

也就是说输入的字符串的第四位

嗯...貌似fs:0x28是类似"金丝雀"的东西, 用来检查有没有被覆盖的......检查的地点在0x18(%rsp).

看一下0x40245e这个地方是一个字符串"flyers".

![img](https://mf3qv90vh2.feishu.cn/space/api/box/stream/download/asynccode/?code=Njg0MjczMDFjMjVhYzk4ZTc1NmZlM2I2OWNmZjhkMGVfVmtyUFRBdnFMRzRDeWMyRFpUTWZnSkNtNXFOV3JLMlFfVG9rZW46Ym94Y25qNVRqalRnSHdUNVBsdm1BR0N5dGhiXzE2MzIyOTUyMDU6MTYzMjI5ODgwNV9WNA)

也就是说, 我们输入的字符串提供的是偏移量, 每个字符串的低四位要在0x4024b0里面找到"flyers".

"flyers"对应的偏移量是`[9,15,14,5,6,7]`, 对应输入字符的低4位, 

根据acsii表得到的答案是`[105,111,110,101,102,103]`,翻译成acsii码是`ionefg`

### phase_6

好长啊......

```
00000000004010f4 <phase_6>:
  4010f4: 41 56                 push   %r14
  4010f6: 41 55                 push   %r13
  4010f8: 41 54                 push   %r12
  4010fa: 55                    push   %rbp
  4010fb: 53                    push   %rbx
  4010fc: 48 83 ec 50           sub    $0x50,%rsp
  401100: 49 89 e5              mov    %rsp,%r13
  401103: 48 89 e6              mov    %rsp,%rsi
  401106: e8 51 03 00 00        callq  40145c <read_six_numbers>
  40110b: 49 89 e6              mov    %rsp,%r14
  40110e: 41 bc 00 00 00 00     mov    $0x0,%r12d
  401114: 4c 89 ed              mov    %r13,%rbp
  401117: 41 8b 45 00           mov    0x0(%r13),%eax
  40111b: 83 e8 01              sub    $0x1,%eax
  40111e: 83 f8 05              cmp    $0x5,%eax
  401121: 76 05                 jbe    401128 <phase_6+0x34>
  401123: e8 12 03 00 00        callq  40143a <explode_bomb>
  401128: 41 83 c4 01           add    $0x1,%r12d
  40112c: 41 83 fc 06           cmp    $0x6,%r12d
  401130: 74 21                 je     401153 <phase_6+0x5f>
  401132: 44 89 e3              mov    %r12d,%ebx
  401135: 48 63 c3              movslq %ebx,%rax
  401138: 8b 04 84              mov    (%rsp,%rax,4),%eax
  40113b: 39 45 00              cmp    %eax,0x0(%rbp)
  40113e: 75 05                 jne    401145 <phase_6+0x51>
  401140: e8 f5 02 00 00        callq  40143a <explode_bomb>
  401145: 83 c3 01              add    $0x1,%ebx
  401148: 83 fb 05              cmp    $0x5,%ebx
  40114b: 7e e8                 jle    401135 <phase_6+0x41>
  40114d: 49 83 c5 04           add    $0x4,%r13
  401151: eb c1                 jmp    401114 <phase_6+0x20>
  401153: 48 8d 74 24 18        lea    0x18(%rsp),%rsi
  401158: 4c 89 f0              mov    %r14,%rax
  40115b: b9 07 00 00 00        mov    $0x7,%ecx
  401160: 89 ca                 mov    %ecx,%edx
  401162: 2b 10                 sub    (%rax),%edx
  401164: 89 10                 mov    %edx,(%rax)
  401166: 48 83 c0 04           add    $0x4,%rax
  40116a: 48 39 f0              cmp    %rsi,%rax
  40116d: 75 f1                 jne    401160 <phase_6+0x6c>
  40116f: be 00 00 00 00        mov    $0x0,%esi
  401174: eb 21                 jmp    401197 <phase_6+0xa3>
  401176: 48 8b 52 08           mov    0x8(%rdx),%rdx
  40117a: 83 c0 01              add    $0x1,%eax
  40117d: 39 c8                 cmp    %ecx,%eax
  40117f: 75 f5                 jne    401176 <phase_6+0x82>
  401181: eb 05                 jmp    401188 <phase_6+0x94>
  401183: ba d0 32 60 00        mov    $0x6032d0,%edx
  401188: 48 89 54 74 20        mov    %rdx,0x20(%rsp,%rsi,2)
  40118d: 48 83 c6 04           add    $0x4,%rsi
  401191: 48 83 fe 18           cmp    $0x18,%rsi
  401195: 74 14                 je     4011ab <phase_6+0xb7>
  401197: 8b 0c 34              mov    (%rsp,%rsi,1),%ecx
  40119a: 83 f9 01              cmp    $0x1,%ecx
  40119d: 7e e4                 jle    401183 <phase_6+0x8f>
  40119f: b8 01 00 00 00        mov    $0x1,%eax
  4011a4: ba d0 32 60 00        mov    $0x6032d0,%edx
  4011a9: eb cb                 jmp    401176 <phase_6+0x82>
  4011ab: 48 8b 5c 24 20        mov    0x20(%rsp),%rbx
  4011b0: 48 8d 44 24 28        lea    0x28(%rsp),%rax
  4011b5: 48 8d 74 24 50        lea    0x50(%rsp),%rsi
  4011ba: 48 89 d9              mov    %rbx,%rcx
  4011bd: 48 8b 10              mov    (%rax),%rdx
  4011c0: 48 89 51 08           mov    %rdx,0x8(%rcx)
  4011c4: 48 83 c0 08           add    $0x8,%rax
  4011c8: 48 39 f0              cmp    %rsi,%rax
  4011cb: 74 05                 je     4011d2 <phase_6+0xde>
  4011cd: 48 89 d1              mov    %rdx,%rcx
  4011d0: eb eb                 jmp    4011bd <phase_6+0xc9>
  4011d2: 48 c7 42 08 00 00 00  movq   $0x0,0x8(%rdx)
  4011d9: 00 
  4011da: bd 05 00 00 00        mov    $0x5,%ebp
  4011df: 48 8b 43 08           mov    0x8(%rbx),%rax
  4011e3: 8b 00                 mov    (%rax),%eax
  4011e5: 39 03                 cmp    %eax,(%rbx)
  4011e7: 7d 05                 jge    4011ee <phase_6+0xfa>
  4011e9: e8 4c 02 00 00        callq  40143a <explode_bomb>
  4011ee: 48 8b 5b 08           mov    0x8(%rbx),%rbx
  4011f2: 83 ed 01              sub    $0x1,%ebp
  4011f5: 75 e8                 jne    4011df <phase_6+0xeb>
  4011f7: 48 83 c4 50           add    $0x50,%rsp
  4011fb: 5b                    pop    %rbx
  4011fc: 5d                    pop    %rbp
  4011fd: 41 5c                 pop    %r12
  4011ff: 41 5d                 pop    %r13
  401201: 41 5e                 pop    %r14
  401203: c3                    retq   
```

让r13 = rsp, rsi = rsp, 执行`read_six_numbers`函数, 读入的6个数字应该是在`rsp+0`到`rsp+0x14`上.让r14 = rsp, 让r12d(r12的低32位) = 0x0.

下面的部分大概能看出来是在循环.

(这里是401114)让rbp=r13, 让eax = memory[r13], eax -= 0x1, 比较eax和0x5, 如果大于就爆炸(说明读入的数必须小于等于6, **而且还不能是0** , 这一点我"初见"的时候漏了......因为0-1 = -1(signed) = 一个很大的unsigned, 这里的比较是在unsigned的比较环境下的......). 

让r12d += 1, 比较r12d和0x6, 如果相等则跳转到401153, 否则让ebx = r12d, **(这里是401135)让rax = ebx, 让eax = memory[rsp+4\*rax](这里应该能看出来是读入的第rax个,也就是r12d个数), 然后比较eax和memory[rbp],如果相等的话爆炸(真是奇妙), 不相等的话继续让ebx+=1, 比较ebx和0x5, 如果小于等于就跳转回401135**,否则就让r13+=0x4,跳转到401114.

 这一段循环看似很绕, 其实很简单......首先标黑体的是一个循环, 可以看做一个操作整体, 这样便于查看大的循环.这里大的循环的索引是`r13`或者`r12d`, 小的循环的索引就是`ebx`, 目的是确保每个元素不相等,相当于下面的循环:

```
for(int i = 0; i < 6; i++){
    if((signed)nums[i] - 1> 5) explode_bomb(); 
    for(int j = i + 1; i < 6; j++){
        if(nums[i] == nums[j]) explode_bomb();   
    }
}
```

也就是说每个元素不相等然后每个元素都要小于等于6, 注意到还是用的unsigned的比较指令, 所以输入的数一定属于`1,2,3,4,5,6`



现在从401153开始看,令rsi=0x18+rsp,  让rax=r14(初始值是rsp),  让ecx等于立即数0x7,让edx等于ecx, 然后edx-=memory[rax],然后让memory[rax] = edx, 然后让rax+=0x4, 比较rax与rsi, 不相等就跳转到501160.

这一段很明显,让所有的读入的数num = 0x7 - num. 



现在从40116f开始看, 让esi=0x0, 跳转到401197, 让ecx=memory[rsp+rsi](处理后的第rsi/4个数),比较ecx与立即数0x1, 若小于等于则跳到401183,否则让rax = 0x1, 让edx=0x6032d0, 跳到401176(也就是刚才跳到401197的下一步).让rdx = memory[0x8+rdx], 让eax+=0x1, 比较ecx与eax, 不相等的话跳到401176, 否则跳到401188.

也就是说无论这里的`ecx`(也就是经过之前的处理的第e]rsi/4个数)是多少,都要跳转到401188, 唯一的区别是rdx的值是多少, 若`ecx`小于等于1那么`rdx`,否则rdx会迭代, 2对应rdx的值是0x6032e0, 3对应0x6032f0...一直到6对应0x603320.

![img](https://mf3qv90vh2.feishu.cn/space/api/box/stream/download/asynccode/?code=MWE3OWRmZDUyZDlkOGZkODk2MDhiYmU5ZjNmOGE1MWJfMXh5MnA5R3g4Z3ppQVFqODVGVm5wckhCRjF4U2pVUFJfVG9rZW46Ym94Y25Zc2hEM2E5MW1lUXpGMlNPbWRiSEdYXzE2MzIyOTUyMDU6MTYzMjI5ODgwNV9WNA)

那么401183是啥呢, 让edx=0x6032d0 ,401188是啥呢, 让memory[0x20+rsp+2*rsi] = rdx, 让rsi+=4, 比较rsi与0x18, 若相等则跳到4011ab,否则让ecx=memory[rsp+rsi], 比较ecx与0x1, 若小于等于则跳到401183, 否则让eax = 0x1, 让edx = 0x6032d0, 跳到401176去.......

这里就是把每个数对应节点的地址放在了rsp+0x20这里...



从4011ab开始看, **让**`**rbx**`**=memory[0x20+rsp]**, 让`rax`=rsp+0x28, 让`rsi`=rsp+0x50, 让`rcx`=`rbx`, (这里是4011bd)让`rdx`=memory[rax], 让memory[0x8+rcx] = rdx, 让rax+=0x8, 比较rax和rsi, 如果相等则跳到4011d2, 否则让rcx = rdx, 跳转到4011bd.

所以说这里是遍历了上一步里栈上从0x20+rsp开始的空间(遍历了1-5, 没有6), 记为`b[1]`, `b[2]`啥的话, 比如遍历到`b[i]`, 就去把`memory[b[i-1]+0x8]`改写成`b[i]`......

这里其实类似双指针遍历链表...然后这里这个结构其实就是个链表.`0x8`是`node`节点对于`next`指针的偏移量. 最后的结果就是把原来的链表按照目前rsp+0x20这里的顺序重新排列了.



从4011d2开始看, 让memory[rdx+0x8] = 0x0(也就是`memory[b[6]+0x8]`, 相当于尾结点`next`指针置`null`), 这个时候0x6032d0的链表已经重新排序了, 让`ebp` = 0x5, (这里是4011df)让rax = memory[rbx+0x8](也就是链表中下一个node的地址),让eax = memory[rax](也就是链表中下一个node的值), 比较eax和memory[rbx](也就是目前的node的值), 若小于则爆炸, 否则就让rbx = memory[rbx+0x8], 让ebp-=1, 若`ebp!=0`则跳回4011df, 否则就返回函数(也就意味着炸弹解除).

也就是说这里是要求对于栈上从0x20+rsp开始的`b[1], b2[2], b[3]...b[5]`, `memory[memory[b[i]+0x8]] <= memory[b[i]]`. 根据上一步的变换我们可以知道在0x6032d0那里的情况是`memory[b[i]+0x8] = b[i+1]`. 所以这里的条件等价于`memory[b[i+1]]<=memory[b[i]]`. 更进一步, 就是让排序后的链表递减.



再去看一下0x6032d0的排布,我们可以知道内部的构造是这样的, 每个"单元"占了2*8个字节, 第二个8个字节是下一步的地址, 第一个是我们用来比较的对象,可以看到其值分别为`0x14c, 0xa8, 0x39c, 0x2b3, 0x1dd, 0x1bb`,要保证它们是递减的的排布序列`b`应该是`3,4,5,6,1,2`, 对应的输入字符就是`4 3 2 1 6 5`

![img](https://mf3qv90vh2.feishu.cn/space/api/box/stream/download/asynccode/?code=ZjhjNTBmOTA5MTNiZTkxNjFmYTliOGZjNmZhNDQ2YTRfTGpPU2Q1d3JGRG51Q0liZE1qc0ZCR2ljeGR1SzJQVzJfVG9rZW46Ym94Y25nQ3l0b0daTEhTZVFMWXI2Wkl3VEhoXzE2MzIyOTUyMDU6MTYzMjI5ODgwNV9WNA)

![img](https://mf3qv90vh2.feishu.cn/space/api/box/stream/download/asynccode/?code=NzM0NDQ1YTRmMjY0ZjZiNzNmYTU5YmI0ZmZjMmI5MjJfcG1ocDZ4UFRYekZJSWNDWGNqa3J0Q2pBYkRNeUNpS3hfVG9rZW46Ym94Y25FQTAySTVncDFZanVaQXNYZVZ5aTBnXzE2MzIyOTUyMDU6MTYzMjI5ODgwNV9WNA)

![img](https://mf3qv90vh2.feishu.cn/space/api/box/stream/download/asynccode/?code=YmUwOGQ0ZTNiZjE0ZmY5OGI2NzczN2I0YWQwMzIwZmFfSGlkdjBYV0xybDhmQzBXRjhKeWNCVURxY2laWHhObndfVG9rZW46Ym94Y25tcFRmWEVIUnRPRXVEZmJINkFBaFFQXzE2MzIyOTUyMDU6MTYzMjI5ODgwNV9WNA)



### Answer

```
Welcome to my fiendish little bomb. You have 6 phases with
which to blow yourself up. Have a nice day!
Border relations with Canada have never been better.
Phase 1 defused. How about the next one?
1 2 4 8 16 32
That's number 2.  Keep going!
1 311
Halfway1 there!
7 0
So you got that one.  Try this one.
ionefg
Good work!  On to the next...
4 3 2 1 6 5
Congratulations! You've defused the bomb!
[Inferior 1 (process 22384) exited normally]
```
