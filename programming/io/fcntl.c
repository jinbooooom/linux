#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
int main(void)
{
	int fd, flag;
	if ((fd = open("./assets/cp.txt", O_RDWR | O_APPEND | O_CREAT | O_TRUNC, 00666)) < 0) {
		perror("open");
		exit(-1);
	}
	write(fd, "abcdefg", 7);
	lseek(fd, 0, SEEK_SET);

	flag = fcntl(fd, F_GETFL); 	// 获取已打开的文件属性，存到 flag 中
	flag = flag & ~O_APPEND;  	// 去除 O_APPEND 属性
	fcntl(fd, F_SETFL, flag);	// 将新属性（去除 O_APPEND ）设置到打开的文件中

	write(fd, "12345", 5);		// 第二次写入将覆盖第一次的内容
	return 0;
}
/*
build & run
jinbo@fang:io$ gcc fcntl.c -o exec/fcntl
jinbo@fang:io$ exec/fcntl 
*/

	
