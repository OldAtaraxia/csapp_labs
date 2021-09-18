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

ok

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

- 栈上开0x18的空间

- rcx里写rsp+0xc

- rdx里写rsp+0x8

- rsi里写立即数$0x4025cf(翻译成字符串是"%d %d")

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

那么对应的答案有:(0,207), (1, 311), (2,707),(3,256),(4,389),(5,206),(6,682),(7,327),都是符合条件的
经过测试看到所有的组都满足条件.

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