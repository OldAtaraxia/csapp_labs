Cache lab

缓存未命中的情况:

- Cold miss : 感觉上类似冷启动, 第一次access肯定会miss
- Conflict miss: 对应位置被其它确实应该占同一地方的给占了
- Capacity miss: 需要缓存的数据比cache还大, 放不进去...

主要改动`csim.c`和`trans.c`, 编译:

```
make clean
make
```

## Part A Building a cache simulator

其实挺简单的......注意细节

要用到LRU: Least Recently used replacement plicy

`csim.c`接受`valgrind`的memory trace文件作为输入, 模拟这个过程中cache的命中/未命中情况, 输出命中, 未命中和替换的数量.

文件夹下提供了一个可执行文件`csim-ref`, 它通过`-s`, `-E`, `-b`指定`s`, `E`和`b`的值. 任务即是让`csim.c`拥有与`csim-ref`同样的行为.

提示: 

- 通过`getopt`得到参数的值, 通过`fscanf`读取trace files
- 缓存模拟器只关心数据的缓存情况, 所以忽略掉所有的指令装载, 也就是`I`开头的.
- 最后答案要通过`printSummary`来打印
- 假设所有内存都已经对齐了, 所有的请求都不可能穿过块的边界, 所以可以忽略traces文件中的大小属性

![img](https://mf3qv90vh2.feishu.cn/space/api/box/stream/download/asynccode/?code=OTM0NjQ1YjZjNjU0MjBkMjZhMTE0YWE3MTY5MDBmZDZfNlRWNmN6TUJSdm5zQzNITUhNUE90U3NXOEZ2a0s1MW9fVG9rZW46Ym94Y25selMzNHpiSFZWQ1Q5M2Rib1VsbWNFXzE2MzI2NDgxODY6MTYzMjY1MTc4Nl9WNA)