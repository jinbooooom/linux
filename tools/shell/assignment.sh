#!/bin/bash/
# 一个为变量和常量赋值的例子

a=z                 # 将字符 'z' 赋值给变量 a
b="a string"        # 将字符串赋值给 b
b2=a_string         # 如果不加引号，就不能有空格
c="a string and $b" # 变量在 shell 里就是宏展开
d="a string and $c" # 继续宏展开
e=$(ls -a)          # 命令的结果
f=$((5 * 5))        # 算术扩展
g="\t\ta string\n"  # 转义字符

echo $a
echo $b
echo $b2
echo $c
echo $d
echo $e
echo $f
echo $g

dir_name="tt"
mkdir -p ${dir_name}1   # 创建文件夹 ./tt1，花括号确定变量的范围
ls
sleep 3                 # 延迟 3 秒
rm -r ${dir_name}1
ls

