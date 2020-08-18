### 并发服务器
```shell
$ git clone https://github.com/jinbooooom/linux.git
$ cd cd linux/project/tcpServerClient/
$ make # 生成服务器可执行文件
$ make client # 生成客户端可执行文件
```

```shell
# 命令行：可执行文件名 IP地址 输入数字字符串（不要用空格隔开，其它的都可以） 排序算法名称 升序/降序
$ ./client # 客户端输出
Too few parameters.
*************************************************************************************
Usage:
argv[0]: Executable file name.
argv[1]: IP address.
argv[2]: Data.
argv[3]: Which sorting algorithm you want to use.
         [Optional]: insert, shell, select, bubble, quick, merge(default).
argv[4]: Sort ascending or descending?
         [Optional]: des(same with DES and '-'), asc(same with ASC and '+')(default).
*************************************************************************************

We will send default test data.

data:                   9,8,+5,-7,15,6
sort algorithm:         merge
asceding or descending: ASC
Sending message 9,8,+5,-7,15,6 to server...
..sent message.. wait for response..

Response from server:
-7 5 6 8 9 15 

$ ./client 127.0.0.1 5,4,3,-8,99 bubble asc
data:                   5,4,3,-8,99
sort algorithm:         bubble
asceding or descending: asc
Sending message 5,4,3,-8,99 to server...
..sent message.. wait for response..

Response from server:
-8 3 4 5 99 
```


```shell
$ ./server # 对应服务端输出
Accepting connections...
received data from client(127.0.0.1): 9,8,+5,-7,15,6
the data of array is： 9 8 5 -7 15 6 
call merge sort...
after sort：-7 5 6 8 9 15 

received data from client(127.0.0.1): 5,4,3,-8,99
the data of array is： 5 4 3 -8 99 
call bubble sort...
after sort：-8 3 4 5 99 
```

当然，你也可以用 Python 脚本自动生成数据

```shell
$ python data.py # 在刚才客户端所在的终端上运行 Python 脚本
```

