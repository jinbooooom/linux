

# Shell 编程

## 变量

*shell 函数里的变量默认是全局变量，这与 C++ 等语言不同*

*要用局部变量得用 local 修饰*

### \# !/bin/bash

“# !/bin/bash” 用来告诉操作系统，执行该脚本使用的解释器名字。

```shell
#!/bin/bash
# 上面的 “# !/bin/bash” 用来告诉操作系统，执行该脚本使用的解释器名字
echo "Hello world!"
echo "
        I
        Love
        China
"
```

### 赋值

```shell
a=z                 # 将字符 'z' 赋值给变量 a
b="a string"        # 将字符串赋值给 b
b2=a_string         # 如果不加引号，就不能有空格
c="a string and $b" # 变量在 shell 里就是宏展开
d="a string and $c" # 继续宏展开
e=$(ls -a)          # 命令的结果
f=$((5 * 5))        # 算术扩展
let "f2 = $f + $f"    # let 关键字用于数值赋值，类似(())
g="\t\ta string\n"  # 转义字符

dir_name="tt"
# 创建文件夹 ./tt1，花括号确定变量的范围。
# 当然也可以去掉花括号，如果去掉，那么会把 dir_name1（而不是 dir_name）当做变量，而 dir_name1 未定义。
mkdir -p ${dir_name}1   
ls
sleep 3                 # 延迟 3 秒
rm -r ${dir_name}1
ls
```

### unset

unset可以删除一个变量。删除后，变量不能再次使用。不可以删除只读变量。

```shell
name=hahaha
echo "name = $name"
unset name 
echo "name = $name"
#输出
#name = hahahaha
#name = 
```

### 只读变量

```shell
readonly R0=100
R0=200
echo $?   # 上一条指令是错误的，所以 $? 为非0
#输出
#bash: R0：只读变量 #这是报错，因为修改了只读变量的值
#1 报错之后，返回的值不为0，默认返回1，所以用echo $?打印1
```

### 转义与引用

Shell 中一共有 4 中引用符，分别是 双引号，单引号，反引号，转义符

```shell
# 转义
# 跟其他编程语言里的转义一样，使用转义符 \
echo \# 使用转义输出注释符号 \#
Dollar=123
echo \$Dollar is $Dollar
echo 8 \* 8 = 64

# 引用
# Shell 中一共有 4 中引用符，分别是 双引号，单引号，反引号，转义符

# "" 双引号：部分引用，可以解释变量
echo "\$Dollar is $Dollar"
# 带不带双引号看起来一样，但是对于输出空格有区别
VAR="A     B      C"
echo 不带引号对于连续空格只输出一个：$VAR # 输出 A B C
echo "带引号会把所有空格输出：$VAR" # 输出 A     B      C

# '' 单引号：全引用，只按照字面意思输出内容，转义符也不能用了
echo '$Dollar 在单引号内还是 $Dollar。'
echo '转义符在单引号内输出 \，单引号只把内容作为字面量输出'
echo '转义符不能用，单引号内不能输出单引号'

# `` 反引号：命令替换，将命令的标准输出作为值赋给某个变量
# 命令替换也可以使用 $(命令) 的形式
LS=`ls`
echo "=== LS ==="
echo $LS
echo "=== LS ==="
LSA=$(ls -a)
echo $LSA
# $() 支持嵌套
$(echo $(ls) > tmp.sh)
TMP=$(cat tmp.sh)
echo === tmp ===
echo $TMP
echo === tmp ===
rm tmp.sh
```

## 流程控制

### if 

```shell
#!/bin/bash
# bash 的判断与循环与其他语言类似，有 if else elif case

# if 判断结构
# if expression; then
#    command
# elif expression; then
#    command
# else
#    command
# fi
if [ ! -e tmp.sh ];
then
  echo "tmp.sh 不存在，创建它"
  touch tmp.sh

  if [ -e tmp.sh ]; then
    echo "tmp.sh 创建好了"
  else
    echo "tmp.sh 创建失败"
  fi
else
  echo "tmp.sh 存在，删了它"
  rm tmp.sh
fi
```

### test命令返回值与 if 判断的一些区别

test命令结果只是返回命令执行成功还是失败，而不是真值判断。

```shell
[ 1 -eq 1 ]
echo '[ 1 -eq 1 ]' $?  # 0。打印$?只是表示test命令[ 1 -eq 1 ]是否其结果是正确还是false。因为是正确的所以返回0。这与C++等编程语言不一样。
if [ 1 -eq 1 ]; then
    echo "equal" # 满足 if 条件，打印 equal。
else
    echo "not equal"
fi
```

### test 命令

```shell
# 测试结构
# 1. test expression   使用 test 指令
# 2. [expression]      使用 []

# 文件测试
# 1. test file_operator FILE
# 2. [file_operator FILE]
```

#### 文件测试

```shell
# 文件测试符，文件不存在时，均返回假
# -b FILE 当文件存在且是块文件时，返回真，否则为假
# -c FILE 当文件存在且是设备文件时，返回真，否则为假
# -d FILE 测试文件是否为目录
# -e FILE 测试文件或目录是否存在
# -f FILE 测试文件是否为普通文件
# -x FILE 判断文件是否为可执行文件
# -w FILE 判断文件可写
# -r FILE 判断文件可读
# -l FILE 判断是否为链接文件
# -p FILE 判断是否为管道文件
# -s FILE 判断文件存在且大小不为 0
# -S FILE 判断是否为 socket 文件
# -g FILE 判断文件是否设置了 SGID
# -u FILE 判断文件是否设置了 SUID
# -k FILE 判断文件是否设置了 sticky 属性
# -G FILE 判断文件属于有效的用户组
# -O FILE 判断文件属于有效的用户
# FILE1 -nt FILE2 FILE1 比 FILE2 新时返回真
# FILE1 -ot FILE2 FILE1 比 FILE2 旧时返回真
```

```shell
# 如果目录不存在就创建。在linux下目录也是文件
if [ ! -d $result_name ];then
   mkdir -p $result_name
fi
```

#### 字符串测试

```shell
# 字符串测试
# 主要包括 等于、不等于、大于、小于、是否为空
# -z string 为空时返回真
echo "字符串测试"
[ -z "" ]
echo '[ -z "" ]' $?   # 结果 0，表示为真

# -n string 非空时返回真
[ -n "aaa" ]
echo '[ -n "aaa" ]' $? # 0
[ "string1" = "string2" ]
echo '[ "string1" = "string2" ]' $? # 1
[ "string1" != "string2" ]
echo '[ "string1" != "string2" ]' $? # 0
[ "string1" > "string2" ]
echo '[ "string1" > "string2" ]' $?  # 0
[ "string1" < "string2" ]
echo '[ "string1" < "string2" ]' $? # 0
```

#### 整数比较

```shell
# 整数比较
# -eq 意 相等
# -gt 意 >
# -lt 意 <
# -ge 意 >=
# -le 意 <=
# -ne 意 !=
[ 1 -eq 2 ]
echo '[ 1 -eq 2 ]' $? # 1
[ 1 -gt 2 ]
echo '[ 1 -gt 2 ]' $? # 1
[ 1 -lt 2 ]
echo '[ 1 -lt 2 ]' $? # 0
[ 1 -ge 2 ]
echo '[ 1 -ge 2 ]' $? # 1
[ 1 -le 2 ]
echo '[ 1 -le 2 ]' $? # 0
[ 1 -ne 2 ]
echo '[ 1 -ne 2 ]' $? # 0

if (( 1 < 2 )); then # 注意双层圆括号，因为 (())用于执行算数真值测试
    echo "less than"
else
    echo "greater then"
fi
# 等价于
if [ 1 -lt 2 ]; then
    echo "less than"
else
    echo "greater then"
fi
```

#### 逻辑测试运算符

```shell
# 逻辑测试符与逻辑运算符
# ! expression 取反
# expression -a expression 同为真，结果为真        
# expression -o expression 只要有一个为真，结果为真
touch tmp.sh
[ ! -e tmp.sh ]
echo '[ ! -e tmp.sh ]' $? # 1

[ -e tmp.sh -a -e tmp1.sh ] 
echo '[ -e tmp.sh -a -e tmp1.sh ]' $? # 1

[ -e tmp.sh -o -e tmp1.sh ]
echo '[ -e tmp.sh -o -e tmp1.sh ]' $? # 0

# -a -o 可以用 && 和 || 替代，不过写法上会有区别
[ -e tmp.sh ] && [ -e tmp1.sh ]
echo '[ -e tmp.sh ] && [ -e tmp1.sh ]' $? # 1

[ -e tmp.sh ] || [ -e tmp1.sh ]
echo '[ -e tmp.sh ] || [ -e tmp1.sh ]' $? # 0
```

### case

```shell
# case 判断结构
# case VAR in
# var1) command ;;
# var2) command ;;
# ...
# *) command ;;
# esac

# 执行时找到第一个匹配的分支并执行相应的命令，然后直接跳到esac之后，不需要像C语言一样用break跳出。

read -p "请输入数字：" NUM
case $NUM in
1) echo "输入为 1" ;;
2) echo "输入为 2" ;;
*) echo "输入为 其他" ;;
esac
```

### 循环

循环控制，break continue，与其他编程语言一致。

#### for

##### for in

```shell
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
# $@：表示脚本参数的内容(即 a b c)
# $#:表示脚本参数的个数(即3)。
for VAR
do
  echo $VAR
done
```

##### 类C的for循环

```shell
echo "类 C 的 for 循环"
# for ((exp1; exp2; exp3))
# do
#   command
# done
for ((i=0, j=100; i < 10; ++i)) # ++i 与 i++ 一样
do
  echo $i $j
done
```

#### while

```shell
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
```

while 按行读取文件

```shell
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
```

#### until

until 与 while 类似，区别在于 until 判断为否，会继续循环，而 while 判断为真，才继续循环

until ((0)) 会无限循环

```shell

COUNT=0
until [ $COUNT -gt 5 ]
do
  echo $COUNT
  let "COUNT++"
done
```

#### select

select 是一种菜单式的循环方式，语法结构与 for 相似，每次循环的值由用户选择。

注意是选择编号。

```shell
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

打印
choose your menu:
1) apple
2) banana
3) orange
4) exit
#? 1
you choose apple
choose again
#? 4
you choose exit
```

## 函数

```shell
# 函数的定义
function FUNCTION_NAME() {
  command
}
省略 function 关键字
FUNCTION_NAME() {
  command
}  
```

当然 function关键字可以省略。如果有了function关键字，用sh运行脚本会出错：function: not found。用bash没有问题。

```shell
# 函数参数
# 与脚本的参数使用一致
func4 () {
  echo "第一个参数 $1"
  echo "第二个参数 $2"
  echo "所有参数 $@"
  echo "参数数量 $#"
}
func4 a b c

打印
第一个参数 a
第二个参数 b
所有参数 a b c
参数数量 3
```

```shell
pow() {
  let "r=$1**$2"
  return $r

}
pow 2 5
echo $? # 32 = 2^5
```

## 数组

bash 只支持一维数组。Shell 数组元素的下标由0开始。

### 赋值

```shell
declare -a mArray
mArray[0]="nihao"
mArray[1]=2
# 定义时赋值，数组的元素用空格分开，其他字符会被当成值，比如 "php", 会被当成 php,
declare -a mArray=("php" "python" 123)
# mArray=("php" "python" 123) # 数组声明也可以忽略 declear -a

# 数组取值，需要用 ${数组名[索引]} 语法
echo ${mArray[0]} # php
echo ${mArray[1]} # python
echo ${mArray[2]} # 123
# 使用 @ * 可以索引全部元素
# @ 得到以空格分开的元素值
# * 得到整个字符串
echo ${mArray[@]} # php python 123
echo ${mArray[*]} # php python 123
```

### 数组长度

```shell
# 数组长度
echo "数组长度是 ${#mArray[@]}" # 数组长度是 3
echo "数组长度是 ${#mArray[*]}" # 数组长度是 3
```

### 截取与合并

```shell
# 数组截取
# 可以获取子数组，下面示例为获取数组的第 1、2 下标位置的元素
echo ${mArray[@]: 1:2} # python 123
# 可以获取数组中某个元素的若干字符，下面示例为获取数组中第二个元素的 从0开始 3个字符
echo ${mArray[1]: 0:3} # pyt

# 合并数组
Front=("javascript" "typescript")
Conn=(${mArray[@]} ${Front[@]})
echo ${Conn[@]} # php python 123 javascript typescript
# 合并得到数组的长度
echo ${#Conn[@]} # 5       
```

### 取消数组元素

需要注意的是，数组被取消位置的元素变为了空，而不是后面的元素向前移动。

```shell
# 取消数组或元素
echo "原始数组：${mArray[@]}"
unset mArray[1]
echo "取消下标为 1 的元素后，数组为：${mArray[@]}，数组长度为 ${#mArray[@]}"
# 需要注意的是，数组的 1 位置的元素变为了空，而不是后面的元素向前移动
echo "数组 0 位置的元素为 ${mArray[0]}， 
数组 1 位置的元素为 ${mArray[1]}， 
2 位置的元素为 ${mArray[2]}"

#打印
原始数组：php python 123
取消下标为 1 的元素后，数组为：php 123，数组长度为 2
数组 0 位置的元素为 php， 
数组 1 位置的元素为 ， 
2 位置的元素为 123
```

# 编程示例
