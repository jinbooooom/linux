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

x=5
if [ $x = 5 ]; then # 中括号两边必须有空格，不然报错
    echo "x = 5"
else
    echo "x != 5"
fi

ANSWER=maybe
echo $ANSWER

# "$ANSWER" 其实可以不加双引号，但考虑到 ANSWER 为空 shell 命令会出错，用引号括起来保证操作符后总有一个字符串
if [ -z "$ANSWER" ]; then       
    echo "there is no answer." >&2
    exit 1
fi

if [ "$ANSWER" = "yes" ]; then
    echo "the answer is no."
elif [ "$ANSWER" = "maybe" ]; then
    echo "the answer is maybe."
else    
    echo "the answer is unknown."
fi

# 测试文件是否存在
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

echo "case 判断结构"
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

rm tmp.sh