#!/bin/bash

# shell function demo

f1() 
{
    echo "call f1()"
    a=5
    global b=6
    return
}

f2() 
{
    echo "call f2()"
    echo $a         # shell 函数里的变量默认是全局变量，这与 C++ 等语言不同
    return
}

echo "in main"
f1
f2
echo $b              # 以为 b 是 f1() 里的局部变量，在外面打印它，b 不是 6 而是空