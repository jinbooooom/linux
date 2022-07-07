#!/bin/bash
# 函数

# 函数的定义
# function FUNCTION_NAME() {
#   command
# }
# 省略 function 关键字
# FUNCTION_NAME() {
#   command
# }            
function func1 {
  echo 1 
}
func2() {
  echo 2 
}

# 函数调用
func1
func2

# 函数返回值
func3 () {
  echo '请输入函数的返回值:'
  read N
  return $N
}
func3
echo "上个函数的返回值是" $?   # 使用 $? 获取上一条指令的返回值

# 函数参数
# 与脚本的参数使用一致
func4 () {
  echo "第一个参数 $1"
  echo "第二个参数 $2"
  echo "所有参数 $@"
  echo "参数数量 $#"
}
func4 a b c

# 使用 set 可以指定位置的脚本（或函数）参数值
func5() {
  set q w e
  echo "参数1 $1"
  echo "所有参数: $@"
}
func5

# 移动位置参数：在 Shell 中可以使用 shift 命令把参数左移一位
func6() {
  while [ $# -gt 0 ]
  do
    echo current \$1 is $1
    shift
  done
}
func6 q w e r t

# 实现一个 pow 函数
pow() {
  let "r=$1**$2"
  return $r

}
pow 2 5
echo $?