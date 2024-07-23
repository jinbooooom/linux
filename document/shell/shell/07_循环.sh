#!/bin/bash
# 循环
# Shell 的循环主要有 for、while、until、select 几种

# for 循环
# 带列表的 for 循环：
# for VAR in (list)
# do
#   command
# done
for NUMBER in 1 2 3 4 5
do
  echo $NUMBER
done

fruits="apple banana orange"
for FRUIT in ${fruits}
do
  echo $FRUIT
done

# 循环数字时可以使用 {a..b} 表示从 a 循环到 b
for N in {2..10}
do
  echo $N
done
# 其中 {2..10} 可以用 seq 命令替换
echo "echo with seq:"
for N in $(seq 2 10)
do
  echo $N
done
# seq 命令可以加 “步长”
for N in $(seq 1 2 20)
do
  echo $N
done

# 可以看出，for in 后面的内容可以是任意命令的标准输出
# 比如，我们可以输出当前目录下的所有带 sh 的文件
for VAR in $(ls | grep sh)
do
  echo $VAR
done

# 如果 for 后面没有 in ，则相当于是 in $@
# 你可以执行 bash 07_循环.sh a b c 试一试。
echo $@ # 表示脚本参数的内容
echo $# # 表示脚本参数的个数。
for VAR
do
  echo $VAR
done


echo "类 C 的 for 循环"
# for ((exp1; exp2; exp3))
# do
#   command
# done
for ((i=0, j=100; i < 10; ++i)) # ++i 与 i++ 一样
do
  echo $i $j
done


echo "while 循环"
# 语法如下：
# while expression
# do
#   command
# done
# while ((1)) 会无限循环
COUNT=0
while [ $COUNT -lt 5 ]
do
  echo $COUNT
  let "COUNT++"
done

# while 按行读取文件
echo "john  30  boy
sue   20  girl" > tmp.txt
while read LINE
do
  NAME=`echo $LINE | awk '{print $1}'`
  AGE=`echo $LINE | awk '{print $2}'`
  SEX=`echo $LINE | awk '{print $3}'`
  echo $NAME $AGE $SEX
done < tmp.txt    # 输入重定向
rm tmp.txt


echo "until 循环"
# until 与 while 类似，区别在于 until 判断为 否，会继续循环，而 while 判断为 真，才继续循环
# until ((0)) 会无限循环
COUNT=0
until [ $COUNT -gt 5 ]
do
  echo $COUNT
  let "COUNT++"
done


echo "select 循环"
# select 是一种菜单式的循环方式，语法结构与 for 相似，每次循环的值由用户选择。
# 注意是选择编号
echo "choose your menu:"
select MENU in "apple" "banana" "orange" "exit"
do
  echo "you choose $MENU"
  if [[ $MENU == "exit" ]]
  then
    break
  else
    echo "choose again"
  fi
done

# 循环控制，break continue，与其他编程语言一致