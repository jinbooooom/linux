/*
 * 《操作系统导论》５.1
 * fork 系统调用示例程序
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    printf("hello world (pid:%d)\n", (int)getpid());
    int rc = fork();
    if (rc < 0)
    {
        fprintf(stderr, "fork failed\n");
        exit(1);
    }
    else if (rc == 0)  // 子进程的返回值是 0
    {
        printf("hello, I am child (pid:%d)\n", (int)getpid());  // getpid() 获取目前进程的进程描述符 PID
    }
    else if (rc > 0)  // 父进程的返回值是子进程的进程描述符
    {
        printf("hello, I am parent of %d (pid:%d)\n", rc, (int)getpid());
    }
    return 0;
    // 在子进程里执行 if(rc == 0) 的内容
    // 在父进程里执行 if(rc > 0) 的内容
    // 不要把它当做一个程序来看
}

/*
jinbo@fang:~/gitme/linux/programming/multiprocessing$ gcc -o fork fork.c
jinbo@fang:~/gitme/linux/programming/multiprocessing$ ./fork
hello world (pid:17883)
hello, I am parent of 17884 (pid:17883)
hello, I am child (pid:17884)
*/