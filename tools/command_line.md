## 简单命令

- date  打印日期与时间
- cal  打印日历
```shell
jinbo@fang:~/gitme/linux/test$ cal
      八月 2020         
日 一 二 三 四 五 六  
                   1  
 2  3  4  5  6  7  8  
 9 10 11 12 13 14 15  
16 17 18 19 20 21 22  
23 24 25 26 27 28 29  
30 31 
```
- df 查看磁盘驱动器的可用空间

- pwd  打印当前目录

- watch -n 5 nvidia-smi  # 查看显卡信息，每隔五秒刷新一次显示

- cd 切换目录

- touch xx.md 创建空文件 xx.md

- top 动态显示当前耗费资源最多的进程信息

- ping 测试网速

- clear 清屏

- reboot 重启

- shutdown 关机

- netstat 显示网络相关信息

- ifconfig 显示或配置网络设备
## linux 常用命令
### ls
- ls | wc -w :查看当前目录下有多少个文件及文件夹（不包括子文件夹）
	- wc 统计文本中行数、字数、字符数，例如 `wc xx.md`（也等于 `wc -lwc xx.md`），打印 xx.md 文件里的行数、单词数、字符数
- ls ~/ 列出来 home 下的文件夹，也等于 `ls /home/username`
- ls -la 以长格式(-l)列出所有的(-a)文件夹，也可以写为 ls -l -a
	- ls -laF 用长格式列出所有的文件夹并且在每一个列出的名字后面加上类型指示符（类如是目录名字后面会加一个斜杠），也可写成 ls -l -a -F
- ls -R  连同子目录的内容一起列出（递归列出），等于该目录下的所有文件都会显示出来。也可以直接用 tree 命令(需要先安装 sudo apt install tree)
```shell
jinbo@fang:~/gitme/linux$ ls -l
总用量 52
-rw-rw-r-- 1 jinbo jinbo 35149 7月  31 19:25 LICENSE
-rw-rw-r-- 1 jinbo jinbo   242 8月   2 23:47 README.md
drwxrwxr-x 2 jinbo jinbo  4096 8月   3 00:00 src
drwxrwxr-x 5 jinbo jinbo  4096 8月   3 00:21 test
drwxrwxr-x 2 jinbo jinbo  4096 7月  31 19:59 tools
jinbo@fang:~/gitme/linux$ ls | wc
      5       5      33
```
### find

find：这个命令用于查找文件，功能强大。例如：find ./*.txt，查找当前目录及其子目录下所有扩展名是 .txt 的文件。

```shell
find *[[:lower:]123] # 以小写字母或数字1、2、3中的任一个结尾的任一文件
```
### file 用于确定文件的种类
```markdown
# 比如 1.txt 其实是 1.JPEG 重命名的，$ file 1.txt 显示
1.txt: JPEG image data, JFIF standard 1.01, resolution (DPI), density 72x72, segment length 16, baseline, precision 8, 440x440, frames 3
```

### mkdir 创建目录

- -p 若所要建立的目录的上层目录尚未建立，则一并建立上层目录

  ```shell
  jinbo@fang:~/gitme/linux/test$ mkdir -p parent/child
  jinbo@fang:~/gitme/linux/test$ tree
  .
  ├── dir1
  │   ├── dir11
  │   ├── dir12
  │   └── f1a.md
  ├── dir2
  ├── parent
  │   └── child
  └── repo.md
  ```

- `mkdir dir1 dir2 path_to/dir3`  同时创建 dir1 dir2 path_to/dir3

  ```shell
  jinbo@fang:~/gitme/linux/test$ mkdir dir1 dir2 dir1/dir11
  jinbo@fang:~/gitme/linux/test$ tree
  .
  ├── dir1
  │   └── dir11
  ├── dir2
  ├── parent
  │   └── child
  └── repo.md
  ```

### rmdir 删除目录

- `rmdir -p` 删除

### rm 删除文件和目录

- `rm item...`

  - -r 递归删除目录及子目录

  - -v 操作完后显示信息性消息

  - -i 删除一个已存在的文件时，提示用户确认

    ```shell
    jinbo@fang:~/gitme/linux/test$ tree
    .
    ├── dir2
    ├── parent
    └── repo.md
    
    2 directories, 1 file
    jinbo@fang:~/gitme/linux/test$ rm -i repo.md
    rm：是否删除普通文件 'repo.md'？ y
    jinbo@fang:~/gitme/linux/test$ tree
    .
    ├── dir2
    └── parent
    
    2 directories, 0 files
    
    ```

### cp 复制文件和目录

- `cp path_to1/file1 path_to2/file2 file2`若与 file1同名，则覆盖，加上 -i 选项，区别在于覆盖之前通知用户是否覆盖 `cp -i path_to1/file1 path_to2/file2 file2`
- `cp file1 file2 dir1` 把 file1，file2 复制到目录 dir 里，前提 dir 存在
- `cp -r dir1 dir2` 把文件夹 dir1 递归地复制到 dir2（不存在则自动创建）

### mv 移动或者重命名

- -u 只复制那些不存在的文件或更新过的名字(虽文件名改动)
- -i 遇到重复的询问是否覆盖
- -v 移动时显示信息性消息，如 `'repo.md' -> 'dir2/repo.md'`
- -r 操作文件夹及子文件夹

- `mv item1 item2 ` 与 cp 类似，将文件（或目录）item1 移动（或重命名）为 item2
- `mv item... dir`  将一个或多个条目从一个目录移动到另一个目录下

### du 显示目录或文件的大小

- du -h 或 du -h --max-depth=1：

  前者查看各文件夹及其子文件夹大小，后者以当前目录为节点，只往目录树下查找一层，即当前目录下的文件夹（不包括子文件夹）。

### ln 创建链接

- `ln file link `用来创建硬链接
- `ln -s item link` 用来创建符号链接
硬链接不能引用与该链接不在同一个磁盘分区的文件
硬链接无法引用目录
符号链接指向的文件与符号链接自身几乎没有区别，当删除一个符号链接时，删除的只是符号链接而不是文件本身，如果删除了文件而符号链接依旧存在，这就是坏链接。

### 标准输出重定向
I/O 重定向功能可以指定标准输出内容发送到哪里
```shell
ls -rl > tree_output.txt  # > 表明重新写入，会清除之前写的内容
tree >> tree_output.txt		# >> 表明在之前的内容后追加新内容cat
less tree_output.txt 
```

### less

- less filename 可以用来查看文件内容，按 Q 可以退出 less 程序lsls -

### tee

从 stdin 读取数据，并输出到 stdout 和文件
```shell
ls -lR | tee ls.txt  
# 跟标准输出重定向区别既可以在终端显示命令结果也输出结果保存到文件
```

### cat
可以在终端显示比较短的文件而不需要分页，功能像 less，但 less 操作与 vi 差不多
`cat path/xx.txt`
`cat 1.md 2.md` 把想要的文件内容连接在一起显示
`cat 1.md 2.md > my_cat.txt `将连在一起的内容写入到文本，不在终端显示

### head/tail
```shell
head -n 3 xx.txt  # 显示文件前 3 行
tail -n 2 xx.txt  # 显示文件后 2 行	
```

### man 帮助手册

- man ls 查看关于 ls 的帮助信息

### 管道

管道操作符 | 把一个命令的标准输出传送到另一个命令的标准输入中
```shell
格式 command1 | command2 
ls -lr | less
ls -lr | sort | uniq | less  # ls 的结果用 sort 排序，再取里面不重复的行，最终用 less 显示
cat 1a/2a.txt | wc -l -w  
# 1a/2a.txt 的内容是 "hello world!"，
# 该命令打印`      1       2`表示 1 行 2 个单词
```

### 更改文件权限
八进制数字表示法
- 文件类型
- 权限属性
- 权限属性实例
```
drwxrwxr-x 3 jinbo jinbo  4096 11月 26 09:31 1a
---xrwxrwx 1 jinbo jinbo   482 11月 26 09:55 1a.txt
drwxrwxr-x 4 jinbo jinbo  4096 11月 26 13:53 1b
drwxrwxr-x 2 jinbo jinbo  4096 11月 26 14:17 1c
-rwxrwxrwx 1 jinbo jinbo   475 11月 26 09:56 1d.md
drwxrwxr-x 2 jinbo jinbo  4096 11月 26 14:59 1e
-rwxrwxrwx 1 jinbo jinbo 28205 9月  16 10:56 1e.jpeg
-rwxrwxrwx 1 jinbo jinbo    23 11月 26 20:32 hello.py
lrwxrwxrwx 1 jinbo jinbo     2 11月 26 15:00 link -> 1c
-rw-rw-r-- 1 jinbo jinbo   544 11月 26 16:13 ls.txt
-rw-rw-r-- 1 jinbo jinbo   960 11月 26 15:40 my_cat.txt
-rw-rw-r-- 1 jinbo jinbo     7 11月 26 20:30 sh.sh
-rw-rw-r-- 1 jinbo jinbo   800 11月 26 15:27 tree_output.txt
```

```shell
$ chmod 000 1a.txt
$ cat 1a.txt
cat: 1a.txt: 权限不够
$ chmod 777 1a.txt
cat 1a.txt  # 现在可以访问了
$ chmod 000 1a  # 改变文件夹权限
$ less 1a/1a.txt
1a/1a.txt: 权限不够  # 虽然文件的权限是 -rwxrwxrwx，但文件夹的权限是 d---------，文件夹都不能进去，文件自然访问不了
```

符号表示法
u：所有者权限

g：组权限

o：其他所有用户

a：所有用户

u+x/u-x：为文件所有者增加/删除可执行权限

+x，也即 a+x

u+x，go = rx：为文件所有者添加可执行权限，同时设置所属群组和其它用户具有读权限和可执行权限。

### 进程
- ps #显示当前终端进程
- ps x # 显示所有终端所控制的进程，TTY中的?表示没有控制终端，STAT是state的缩写，显示的是当前进程的状态,S表示睡眠状态，进程不在运行，而是在等待。

```
 PID TTY      STAT   TIME COMMAND
 2740 pts/2    Ss     0:00 bash
 2858 ?        Sl     0:00 /usr/lib/x86_64-linux-gnu/deja-dup/deja-dup-monitor
 6990 ?        Sl     0:14 /usr/lib/firefox/firefox
 7061 ?        Sl     0:08 /usr/lib/firefox/firefox -contentproc -childID 1 -isForBrowser -prefsL
 7122 ?        Sl     0:00 /usr/lib/firefox/firefox -contentproc -childID 2 -isForBrowser -prefsL
 7153 ?        Sl     0:08 /usr/lib/firefox/firefox -contentproc -childID 3 -isForBrowser -prefsL
 8296 pts/2    R+     0:00 ps -x
```
- top # 动态查询进程信息
**使程序在后台运行**
```shell
$ xlogo  # 显示一个包含X标识的可缩放窗口
# 但是这样 shell 提示符没有返回，这是因为 shell 正在等待着 xlogo 程序结束，若用 Ctrl+C 关闭 xlogo，shell 提示符就返回了。
$ xlogo & # 让 xlogo 在后台运行， shell 返回 [1] 8798
$ ps
 PID TTY          TIME CMD
 2740 pts/2    00:00:00 bash
 8798 pts/2    00:00:00 xlogo
 8811 pts/2    00:00:00 ps
```
**杀死进程**

```shell
kill -2 8798  # 杀死 PID 为 8798 的进程，-2 的效果等同于 ctrl+C，-20的效果等同于 ctrl+z 暂停进程
# kill 默认执行 kill -15， -15 是终止信号
# -9：强制杀死进程，该信号不会发送给目标进程，而是由内核终止了目标进程，
# 当以这种方式被终止，目标进程将没有机会对自己进行清理或者对当前结果进行保存。
# -9 是没有办法的最后选择。
```
### 网络
- ping # 向网络主机发送数据包，用来检测网络连接是否正常
```
ping www.baidu.com
```
### 推荐/参考链接

- [Linux Tools Quick Tutorial](https://linuxtools-rst.readthedocs.io/zh_CN/latest/index.html) 


