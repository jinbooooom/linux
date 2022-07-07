#!/bin/bash/
# 一个为变量和常量赋值的例子

a=z                 # 将字符 'z' 赋值给变量 a
b="a string"        # 将字符串赋值给 b
b2=a_string         # 如果不加引号，就不能有空格
c="a string and $b" # 变量在 shell 里就是宏展开
d="a string and $c" # 继续宏展开
e=$(ls -a)          # 命令的结果
f=$((5 * 5))        # 算术扩展
let "f2 = $f + $f"  # let 关键字用于数值赋值
g="\t\ta string\n"  # 转义字符

echo $a
echo $b
echo $b2
echo $c
echo $d
echo $e
echo $f
echo $f2
echo $g

dir_name="tt"
# 创建文件夹 ./tt1，花括号确定变量的范围。
# 当然也可以去掉花括号，如果去掉，那么会把 dir_name1（而不是 dir_name）当做变量，而 dir_name1 未定义。
mkdir -p ${dir_name}1   
ls
sleep 3                 # 延迟 3 秒
rm -r ${dir_name}1
ls

