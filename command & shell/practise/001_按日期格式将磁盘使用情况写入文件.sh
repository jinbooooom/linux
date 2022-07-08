#!/bin/bash

## 题目

# 请按照这样的日期格式（xxxx-xx-xx）每日生成一个文件，
# 例如生成的文件名为2017-12-20.log， 
# 并且把磁盘的使用情况写到到这个文件中。

d=`date +%F` # 用 date --help 可以知道 %F 指完整日期格式，等价于 %Y-%m-%d
dir=./tmp/disklog
filename=${dir}/${d}.log
echo $filename
if [ ! -d $dir ]
then
    mkdir -p $dir
fi
df -h > $filename

sleep 5
find $dir -name ${d}.log | xargs rm
echo "delete $filename"