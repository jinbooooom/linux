#!/bin/bash

## 题目

# 找到./tmp/test目录下所有后缀名为.txt的文件

#   1. 批量修改.txt为.txt.bak
#   2. 把所有.bak文件打包压缩为005.tar.gz
#   3. 批量还原文件的名字，即把增加的.bak再删除

# #### 核心要点

# * find用来查找所有.txt文件
# * tar打包一堆文件
# * 还原文件名用for循环

dir="./tmp/test"
find $dir -type f -name "*.txt" > ${dir}/txt.list 
for f in `cat ${dir}/txt.list`
do
    echo $f
    mv $f $f.bak
done

#find /123/ -type f -name *.txt |xargs -i mv {} {}.bak 
#find /123/ -type f -name *.txt -exec mv {} {}.bak \;

for f in `cat ${dir}/txt.list`
do
    echo $f.bak
done > ${dir}/txt.bak.list 

tar -czvf ${dir}/005.tar.gz `cat ${dir}/txt.bak.list | xargs` # 与下面这一行相同
#cat ${dir}/txt.bak.list | xargs tar -czvf ${dir}/005.tar.gz

for f in `cat ${dir}/txt.list`
do 
    mv $f.bak $f
done 


# 清理工作
tar -xzvf ${dir}/005.tar.gz # 也会打印

sleep 5

rm ${dir}/txt.list
rm ${dir}/txt.bak.list
find ${dir} -name "*.bak" | xargs rm