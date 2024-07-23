#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int fd;
    char buf[100];
    if ((fd = open(argv[1], O_RDONLY)) < 0)  // 只读打开文件
    {
        perror("open");
        exit(-1);
    }
    read(fd, buf, 1);              // 从 fd 中读一个字符存到 buf
    write(STDOUT_FILENO, buf, 1);  // 从 buf 中读一个字符写到终端
    lseek(fd, 2, SEEK_CUR);  // 文件偏移量在当前的基础上+2，考虑到之前已经偏移1了，所以相对于文件起始位置，一共偏移3
    read(fd, buf, 1);
    write(STDOUT_FILENO, buf, 1);
    lseek(fd, -1, SEEK_END);  // 文件偏移量为倒数第一个
    read(fd, buf, 1);
    write(STDOUT_FILENO, buf, 1);
    lseek(fd, 0, SEEK_SET);  //文件偏移量置 0
    read(fd, buf, 1);
    write(STDOUT_FILENO, buf, 1);
    close(fd);
    printf("\n");
}

// build：gcc lseek.c -o exec/lseek
// run：exec/lseek assets/test.txt
