#!/bin/bash
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
