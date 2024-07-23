#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>

#define BUF_SIZE 100
void error_handling(char *message);

int main(int argc, char *argv[])
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    struct timeval timeout;
    fd_set reads, cpy_reads;

    socklen_t adr_sz;
    int fd_max, str_len, fd_num, i;
    char buf[BUF_SIZE];
    if (argc != 2)
    {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() error");
    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    /**
     * @brief Construct a new fd zero object
     * 在 fd_set 变量中注册或更改值的操作都由下列宏完成。
    - `FD_ZERO(fd_set *fdset)`：将 fd_set 变量所指的位全部初始化成0
    - `FD_SET(int fd,fd_set *fdset)`：在参数 fdset 指向的变量中注册文件描述符 fd 的信息
    - `FD_CLR(int fd,fd_set *fdset)`：从参数 fdset 指向的变量中清除文件描述符 fd 的信息
    - `FD_ISSET(int fd,fd_set *fdset)`：若参数 fdset 指向的变量中包含文件描述符 fd 的信息，则返回「真」。即要监视的fd是否改变了？
     * 
     */
    FD_ZERO(&reads);
    FD_SET(serv_sock, &reads); //注册服务端套接字
    fd_max = serv_sock;

    while (1)
    {
        cpy_reads = reads;
        timeout.tv_sec = 5;
        timeout.tv_usec = 5000;

        /**
         * @brief select
         * 本来 select 函数只有在监视文件描述符发生变化时才返回。如果未发生变化，就会进入阻塞状态。
         * 指定超时时间就是为了防止这种情况的发生。
         * 不过这种情况下， select 函数返回 0 。因此，可以通过返回值了解原因。如果不向设置超时，则传递 NULL 参数。
         * 
         * fd_max 加 1 是因为文件描述符的值是从 0 开始的
         * 注意 select 会修改 cpy_reads 的值，所以做一个拷贝。
         */
        if ((fd_num = select(fd_max + 1, &cpy_reads, 0, 0, &timeout)) == -1) //开始监视,每次重新监听
            break;
        if (fd_num == 0) // select 未检测到文件描述符发生变化，但是由于超时，所以返回 0
            continue;

        for (i = 0; i < fd_max + 1; i++)
        {
            if (FD_ISSET(i, &cpy_reads)) //查找发生变化的套接字文件描述符
            {
                if (i == serv_sock) //如果是服务端套接字时,受理连接请求
                {
                    adr_sz = sizeof(clnt_adr);
                    clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &adr_sz); // accept 在此

                    FD_SET(clnt_sock, &reads); //注册新连接的 clnt_sock
                    if (fd_max < clnt_sock)
                        fd_max = clnt_sock; // 更新 fd_max，在while循环开始时，调用select，重新更新fd_max
                    printf("Connected client: %d \n", clnt_sock);
                }
                else //不是服务端套接字时
                {
                    str_len = read(i, buf, BUF_SIZE); //i指的是当前发起请求的客户端
                    if (str_len == 0) // 判断来的字符串是正常的数据还是断开连接的EOF。
                    {
                        FD_CLR(i, &reads);
                        close(i);
                        printf("closed client: %d \n", i);
                    }
                    else
                    {
                        write(i, buf, str_len); // 回声服务
                    }
                }
            }
        }
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

/**
 * @brief 
 * 用select只改变了服务端，客户端不需要做任何修改
 */
