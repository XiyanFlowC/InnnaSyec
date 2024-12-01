# InnnaSyec
A reversing tool for mips r5900 (Emotion Engine, aka. TX79).

This tool is able to patch an ELF partically.

MIPS R5900（又称TX79）的逆向辅助工具。

设计为可以部分更新（打补丁）二进制文件的形式。

## Build
Firstly, you need to build simpgen:

如果正在使用 Linux，请输入：
```bash
make all
```
以完成构建。

如果是Windows，请继续阅读。

构建项目。
```batch
cl prtasm.cpp inscodec/codec.c inscodec/disas.c  /O2 /utf-8
```

If you need help, just use:

若需要使用帮助，键入：
```bash
./prtasm --help
```

此外，如果有预处理器指令，请额外构建：
```bash
g++ preprocessor.cpp -o preprocessor
```

## Usage
以所给例子（目前仅为DEBUG编写了样例）：
```bash
preprocessor test.txt test.pp.txt # 预处理test.txt
prtasm -m a -i SLPS_256.04 -o test.elf -s test.pp.txt # 执行汇编
```

## Script Syntax
脚本语法很类似普通的汇编指令。但是需要注意，此工具非常朴素，不支持任何计算，因此类似“addi a0, (FooBar - 0x1000)”的指令无法处理。

此外，为了应对在程序中间打补丁时，可能产生的越界问题，提供了指令.loc SET_ADDR,WARN_ADDR，这样，从这条指令开始产生的指令会从SET_ADDR开始填充，而一旦地址触及WARN_ADDR，此工具会给出警告。（可以通过开关 -r 将之视作致命错误）

预计在未来加入预处理器的功能。

注意一些看上去类似的文法实际上有着不同的含义。

```
; patch at codes point 1
.loc 100fac,1010ff ; 在 100fac 开始写入，到达 1010ff 时警告
f_00:
move    a0, zero
la      a1, str_00
jal     0x6ff2f0
li      a2, 2
beq     zero, zero, f_00
nop
.align 8 ; 对齐到下一个 8 字节对齐位
.galign 8 ; 从此开始的字符串末尾都以 8 字节对齐（填充 0 直至对齐）
str_00:
.asciiz "ERROR: Stuck!\n"

.loc fff000 ; 在 fff000 开始写入
jr      ra
move    v0, zero
```

## MISC

那堆其他文件是什么？

答：没写完的分析器（）不要管。只需要inscodec 和prtasm就好。之后可能移动到单独仓库。
