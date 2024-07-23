#!/bin/bash
# 测试：程序运行过程中经常需要根据实际情况执行特定的命令，
# 比如，判断某个文件是否存在，如果不存在，可能需要先创建该文件
# ls tmp.sh
# echo $?

# 测试结构
# 1. test expression   使用 test 指令
# 2. [expression]      使用 []

# 文件测试
# 1. test file_operator FILE
# 2. [file_operator FILE]
test -e tmp.sh
echo $?         # 不存在，上一个指令结果为 1。1 即为错误
[ -e tmp.sh ]
echo $?

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

rm tmp.sh
rm string2

[ 1 -eq 1 ]
echo '[ 1 -eq 1 ]' $?
if [ 1 -eq 1 ]; then
    echo "equal"
else
    echo "not equal"
fi


