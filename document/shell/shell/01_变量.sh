#!/bin/bash
# 变量
echo "01_变量.sh"

# 变量定义与赋值，等号两边不能用空格分开
name=hahahaha
echo $name
echo 也使用 {} 输出为 ${name}

# 一些特殊变量
test_func() {
  echo "function name is ${FUNCNAME}"
}
test_func

echo "hostname: $HOSTNAME"
echo "hosttype: $HOSTTYPE"
echo "matchtype: $MATCHTYPE"
echo "language: $LANG"
echo "current path: $PWD"
# echo $PATH

# unset可以删除一个变量。删除后，变量不能再次使用。不可以删除只读变量
echo "name = $name"
unset name 
echo "name = $name"

# 只读变量
readonly R0=100
R0=200
echo $?   # 上一条指令是错误的，所以 $? 为非0，一般返回 1

# 变量的作用域
# 变量的作用域又叫“命名空间”，相同名的变量可以在不同命名空间定义。
# 在 Linux 系统中，不同进程 ID 的 Shell 默认为不同的命名空间
VAR_01=100
f1() 
{
  # 在函数内外访问到的是同一个变量
  VAR_01=200
}
f1
echo 变量 VAR_01: $VAR_01

f2() 
{
  # 可以使用 local 关键字声明函数内部的局部变量
  local VAR_01=300
}
f2
echo "local 声明的本地变量不会影响全局变量, VAR_01: ${VAR_01}"

# 子 Shell 不会继承变量
subshell="./temp_subshell.sh"
echo "echo 子 shell 的 VAR_01 为 \$VAR_01" > ${subshell}
bash ${subshell} # 因为子 Shell 不会继承变量，所以打印空

# 导出变量（环境变量），子 Shell 可继承，相当于子 Shell 启动时复制了导出的变量
export VAR_01
bash ${subshell}

# 在子 Shell 中修改 VAR_01 不会影响
sleep 3
rm ${subshell}    # 删除 tmp.sh
echo delete ${subshell} finish