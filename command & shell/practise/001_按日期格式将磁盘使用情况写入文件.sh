#!/bin/bash
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