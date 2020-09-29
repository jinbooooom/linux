#!/bin/bash

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