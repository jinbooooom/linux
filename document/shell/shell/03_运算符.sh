#!/bin/bash
# 运算符
# Shell 的运算符主要有：
# 比较运算符（整数比较），字符串运算符（字符串测试），文件操作运算符（用于文件测试），逻辑运算符，算术运算符，位运算符，自增自减等

# 算术运算符：加减乘除余幂 以及加等，减等，乘等，初等，余等
A=1
B=2
let "C = $A + $B"  # 需要使用 let 关键字执行运算
D = $(($A + $B))
echo C: $C # 4
echo D: $D # 4

# 位运算符：左移 右移 按位与 按位或 按位非 按位异或
var1=1
var2=5

let "var = $var1<<2" # 4
echo var: $var
let "var = $var1&$var2" 
echo var: $var
# 按位非就是 -($var+1)
let "var = ~8"
echo var: $var

# 自增自减，与其他语言类似，分为前置和后置的区别
var1=1
echo "var1 is $var1"
let "var2=++var1"
echo "var2 前置自增 var1，$var2"
var1=1
let "var2=var1++"
echo "var2 后置自增 var1，$var2"

# 其他算术运算
# 使用 $[] 做运算：$[] 和 $(()) 类似，可用于简单的算术运算
echo '$[1+1]' is $[1+1]
echo '$[5 ** 2]' is $[5 ** 2]

# 使用 expr 做运算：使用 expr 要求操作数和操作符之间用空格分开，否则会被当成字符串
expr 1+1
expr 1 + 1
expr 2 \* 2   # 特殊字符运算符需要转义

# 算术扩展： $((算术表达式))
echo $((2*(1+1)))

# 使用 bc 做运算
# 前面介绍的运算只能基于整数，如果想要计算高精度小数，可以使用 Linux 下的 bc 工具。
# bc 是一款高精度计算语言，支持顺序执行，判断，循环，函数等，下面是一个简单的例子
NUM1=1.2
NUM2=2.3
SUM=$(echo "$NUM1+$NUM2" | bc)
echo $SUM
# 你也可以直接在命令行下输入 bc，然后回车进入 bc 命令行模式