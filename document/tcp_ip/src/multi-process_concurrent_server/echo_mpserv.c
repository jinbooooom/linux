#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 30
void error_handling(char *message);
void read_childproc(int sig);

int main(int argc, char *argv[])
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;

    pid_t pid;
    struct sigaction act;
    socklen_t adr_sz;
    int str_len, state;
    char buf[BUF_SIZE];
    if (argc != 2)
    {
        printf("Usgae : %s <port>\n", argv[0]);
        exit(1);
    }
    act.sa_handler = read_childproc; //防止僵尸进程。处理僵尸进程的函数，SIGCHLD 信号被触发就执行。
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    // sigaction 完全可以代替 signal 函数来注册信号处理器。且 sigaction 更稳定。
    // signal 在 Unix 系列的不同系统里可能有区别，但 sigaction 函数完全相同。
    // 子进程终止时会向父进程发送SIGCHLD信号，告知父进程回收自己，但该信号的默认处理动作为忽略，因此父进程仍然不会去回收子进程，需要捕捉处理实现子进程的回收；
    state = sigaction(SIGCHLD, &act, 0);         //注册信号处理器, 当子进程结束时，执行函数 read_childproc 清理僵尸进程
    // PF 是 Protocol Family 的缩写
    serv_sock = socket(PF_INET, SOCK_STREAM, 0); //创建服务端套接字，PF_INET 指 IPV4 互联网协议族，SOCK_STREAM 指 TCP。
    memset(&serv_adr, 0, sizeof(serv_adr));
    // AF 是 Address Family 的缩写
    serv_adr.sin_family = AF_INET; // AF_INET 指 IPV4 地址族
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    // 将 sockaddr_in 型结构体 serv_adr 转换成 sockaddr 型，是因为 sockaddr是给操作系统用的，程序员不应操作；
    // 程序员要做的是用sockaddr_in来表示地址，sockaddr_in区分了地址和端口，使用更方便。
    if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1) //分配IP地址和端口号
        error_handling("bind() error");
    if (listen(serv_sock, 5) == -1) //进入等待连接请求状态
        error_handling("listen() error");

    while (1)
    {
        adr_sz = sizeof(clnt_adr);
        clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &adr_sz);
        if (clnt_sock == -1)
            continue;
        else
            puts("new client connected...");
        pid = fork(); //此时，父子进程分别带有一个套接字
        if (pid == -1)
        {
            close(clnt_sock);
            continue;
        }

        if (pid == 0) //子进程运行区域,此部分向客户端提供回声服务
        {
            close(serv_sock); //关闭服务器套接字，因为从父进程传递到了子进程
            while ((str_len = read(clnt_sock, buf, BUFSIZ)) != 0)
                write(clnt_sock, buf, str_len);

            close(clnt_sock);
            puts("client disconnected...");
            return 0;
        }
        else // 父进程
            close(clnt_sock); //通过 accept 函数创建的套接字文件描述符已经复制给子进程，因此服务器端要销毁自己拥有的
        // 父子进程都有文件描述符 clnt_sock，要两个文件描述符都 close，socket 才被真正的销毁。
    }
    close(serv_sock);

    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

void read_childproc(int sig)
{
    pid_t pid;
    int status;
    /**
     * @brief 
     * #include <sys/wait.h>
     * pid_t waitpid(pid_t pid, int *statloc, int options);
     * 成功时返回终止的子进程ID 或 0 ，失败时返回 -1
     * pid: 等待终止的目标子进程的ID,若传 -1，则与 wait 函数相同，可以等待任意子进程终止
     * statloc: 与 wait 函数的 statloc 参数具有相同含义
     * options: 传递头文件 sys/wait.h 声明的常量 WNOHANG ,即使没有终止的子进程也不会进入阻塞状态，而是返回 0 退出函数。
     * 
     * wait 函数会引起程序阻塞，waitpid 函数不阻塞。
     */
    pid = waitpid(-1, &status, WNOHANG);
    printf("removed proc id: %d \n", pid);
}

/**
 * @brief 《TCP/IP 网络编程》 ch10
 * 
jinbo@fang:multi-process_concurrent_server$ ./server.out 9190
new client connected...
 */
