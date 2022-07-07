#!/bin/bash
# 重定向
# 重定向是指将原本由标准输入输出的内容，改为输入输出的其他文件或设备

# 系统在启动一个进程时，会为该进程打开三个文件：
# 标准输入（stdin）、标准输出（stdout）、标准错误（stderr）
# 分别用文件标识符 0、1、2 标识
# 如果要为进程打开其他的输入输出，需要从整数 3 开始标识
# 默认情况下，标准输入为键盘，标准输出和标准错误为显示器


# 常见的 IO 重定向符号
# > 标准输出覆盖重定向，将命令的标准输出重定向到其他文件中，会直接覆盖原文件内容
# >> 标准输出追加重定向，将命令的标准输出重定向到其他文件中，不会覆盖文件，会在文件后面追加
# >& 标识输出重定向，讲一个标识的输出重定向到另一个标识的输入
# < 标准输入重定向，命名将从指定文件中读取输入，而不是从键盘中读取输入
# | 管道，从一个命令中读取输出，作为另一个命令的输入


# 输出重定向
# 把原本标准输出到屏幕的内容，重定向到 tmp.txt 文件中
echo "result1" > tmp.txt
cat tmp.txt

echo "result2" > tmp.txt
cat tmp.txt

# 输出追加
echo "输出追加："
echo "result3" >> tmp.txt
echo "result3" >> tmp.txt
echo "result3" >> tmp.txt
cat tmp.txt
rm tmp.txt

# 标识输出重定向
echo "未重定向标准错误，会直接输出到页面"
# 制定一个不存在的命令
adhfafahdfakdf > tmp.txt
echo "tmp.txt:" `cat tmp.txt`
rm tmp.txt
echo "重定向标准错误到标准输出，会输出到文件中"
asiiaodfuoaf > tmp.txt 2>&1
echo "tmp.txt:" `cat tmp.txt`

# 标准输入重定向
echo "标准输入重定向："
while read Line
do
  echo $Line
done < tmp.txt

# 管道
# 获取 .sh 文件的名称
ls | grep .sh | cut -f1 -d'.'

# 使用 exec
# exec 是 Shell 的内建命令，执行这个命令时，系统不会启动新的 Shell，而是用被执行的命令替换当前的 Shell 进程
# 因此，在执行完 exec 的命令后，该 Shell 进程将会主动退出
# 例如：执行 exec ls ，后续的其他命令将不会执行。你也可以直接打开 Shell，执行 exec ls 试试
# 此外，exec 还可以用于 I/O 重定向。
# exec < file 将 file 文件中的内容作为 exec 的标准输入
# exec > file 将 file 文件作为标准输出
# exec 3<file 指定文件标识符
# exec 3<&- 关闭文件标识符
# exec 3>file 将写入文件标识符的内容写入到指定文件（输出重定向）
# exec 4<&3 创建文件标识符4，4是3的拷贝 （类似标识输出重定向 2>&1)
# 注：不同的 shell 环境可能会有所差别，比如我在 mac 的 zsh 下就不能正常使用 exec 重定向

# Here Document
# here doc 又称为 此处文档，用于在命令或脚本中按行输入文本。
# 格式为 command << delimiter
# delimiter 是用于标注结束的分隔符

# 示例：
# 你可以在命令行下输入 sort << END 试试
# 你可以在命令行下输入 cat > tmp.txt << END 试试
cat << EOF > tmp.txt
1
2
3
EOF
cat tmp.txt

rm tmp.txt
