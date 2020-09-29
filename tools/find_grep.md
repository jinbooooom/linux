
> Linux下搜索文件内容常用grep，搜索文件信息使用find

## 一、 grep

搜索文本的命令

    命令格式: grep [options]... pattern [file]...
    命令格式: grep 查找规则... 正则表达式 查看文件


### 1.1 查找规则

|options|解释|
|---|---|
|-i|不区分大 小写(只适用于单字符)|
|-r|遍历匹配|
|-v|显示不包含匹配文本的所有行|
|-s|不显示不存在或无匹配文本的错误信息|
|-E|可用于同时匹配多关键词|
|-w|整字匹配|
|-l|查询多文件时只输出包含匹配字符的文件名|
|-c|只输出匹配行的计数|
|-n|显示匹配行及行号|
|-h|查询多文件时不显示文件名|


### 1.2 正则表达式
pattern正则表达式主要参数：

- \： 忽略正则表达式中特殊字符的原有含义。
- ^：匹配正则表达式的开始行。
- $: 匹配正则表达式的结束行。
- \<：从匹配正则表达 式的行开始。
- \>：到匹配正则表达式的行结束。
- [ ]：单个字符，如[A]即A符合要求 。
- [ - ]：范围，如[A-Z]，即A、B、C一直到Z都符合要求 。
- .   ：所有的单个字符。
- \*   ：有字符，长度可以为0.

### 1.3 实例

- 忽略大小写搜索

        grep -i "androiD"  logcat.txt   //从logcat.txt文件中，搜索包含android的文本行，不区分大小写

- 遍历搜索，且不显示无匹配信息

        grep -rs "android" .   //从当前目录下，遍历所有的文件，搜索包含android的文本行

- 整字匹配搜索

        grep -w "android" logcat.txt  //从logcat.txt文件中，搜索包含单词android的文本行
        grep -w "android | ios" logcat.txt  //从logcat.txt文件中，搜索包含单词android或者ios的文本行

- 只列出文件名

        grep -l "android" .

- 统计字符出现次数

        grep -c "android" .

- 显示字符出现所在行

        grep -n "android“
        .
- 显示多条件匹配

        grep -E "android|linux“ .

## 二、 find

搜索文件的命令

    命令格式  find pathname -options [ actions]
    命令格式  find 查找目录  -查找规则 [执行操作]

### 2.1 查找目录

(1) 如果不写，默认为当前路径；
(2) 支持多个路径，目录直接用空格间隔；

    find . -name demo

#### 2.1.1  查看某目录文件个数


```
find . -type f  |wc -l
```

### 2.2 查找规则

#### 2.2.1 根据文件名(name)

`-name`   //根据文件名查找，区分大小写
`-iname`  //根据文件名查找，不区分大小写

通配符说明：
(1)*  匹配任意的若干个字符
(2)?  匹配任意的单个字符
(3)[] 匹配括号内的任意一个字符

    find /data -name dalvi*
    find /data -name dalvik?cache
    find /data -name dalvik-cach[abe]

#### 2.2.2 根据文件类型(type)

- f     普通文件
- d     目录文件
- l     链接文件
- b     块设备文件
- c     字符设备文件
- p     管道文件
- s     socket文件

例如：

    find -type f //查看文件类型

#### 2.2.3 根据目录深度(depth)

- -maxdepth n: 查找最大深度为n
- -mindepth m: 查找最小深度为m


#### 2.2.4 根据文件大小(size)
单位：c(小写), k(小写), M(大写), G(大写)

-size +10M: 查找大于10M的文件
-size -2k: 查找小于2k的文件
-empty: 查找大小为0的文件或空目录

#### 2.2.5 根据文件权限(perm)

例如：

    find -perm 777 //查找权限为777的文件

#### 2.2.6 根据文件所属用户和组

- -user: 根据属主来查找文件
- -group: 根据属组来查找文件

#### 2.2.7  根据uid和gid

- -uid  500: 查找uid是500 的文件
- -gid  1000: 查找gid是1000的文件

#### 2.2.8 根据时间

可以通过`stat`命令来查看文件的时间，下列是按照文件的各种时间来查找文件：

- -mtime  -n  +n: 根据更改(modify)时间，-n指n天以内，+n指n天以前
- -atime  -n  +n: 根据访问(access)时间，-n指n天以内，+n指n天以前
- -ctime  -n  +n: 根据创建(create)时间，-n指n天以内，+n指n天以前
- -mmin   -n  +n: 根据更改(modify)时间，-n指n分钟以内，+n指n分钟以前
- -amin   -n  +n: 根据访问(access)时间，-n指n分钟以内，+n指n分钟以前
- -cmin   -n  +n: 根据创建(create)时间，-n指n分钟以内，+n指n分钟以前

#### 2.2.9  多条件连接

- -a: 两个条件同时满足（and）
- -o: 两个条件满足其一（or）
- -not: 对条件取反（not）

例如，查找当前路径下，以a开头，并排除掉以b结尾的文件或文件夹：

    find -name a* -not -name *b


### 2.3 执行操作

- `-print`   匹配文件输出到标准输出，默认操作
-  `-ls`       查找到的结果，以ls方式显示

        find -name app -ls

- `-ok [command]`     查找完成后，执行command执行，询问执行

        find -name app -ok cat {} \;   //注意：{}前后有空格

- `-exec [command]`   查找完成后，执行command执行，直接执行

        find -name app  -exec ls {} \;
