#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#define BUF_SIZE 100
#define EPOLL_SIZE 50
void error_handling(char *message);

int main(int argc, char *argv[])
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t adr_sz;
    int str_len, i;
    char buf[BUF_SIZE];

    struct epoll_event *ep_events;
    struct epoll_event event;
    int epfd, event_cnt;

    if (argc != 2)
    {
        printf("Usage : %s <port> \n", argv[0]);
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
     * 声明足够大的 epoll_event 结构体数组候，传递给 epoll_wait 函数时，
     * 发生变化的文件描述符信息将被填入数组。
     * 因此，无需像 select 函数那样针对所有文件描述符进行循环。
     */
    epfd = epoll_create(EPOLL_SIZE); //可以忽略这个参数，填入的参数为操作系统参考
    ep_events = malloc(sizeof(struct epoll_event) * EPOLL_SIZE);

    /*
    下面代码将 epfd 注册到 epoll 例程 epfd 中，并在需要读取数据的情况下产生相应事件。接下来给出 epoll_event 的成员 events 中可以保存的常量及所指的事件类型。
    - EPOLLIN：需要读取数据的情况
    - EPOLLOUT：输出缓冲为空，可以立即发送数据的情况
    - EPOLLPRI：收到 OOB 数据的情况
    - EPOLLRDHUP：断开连接或半关闭的情况，这在边缘触发方式下非常有用
    - EPOLLERR：发生错误的情况
    - EPOLLET：以边缘触发的方式得到事件通知。所有 socket 默认都是水平触发监听模式。
    - EPOLLONESHOT：发生一次事件后，相应文件描述符不再收到事件通知。因此需要向 epoll_ctl 函数的第二个参数传递 EPOLL_CTL_MOD ，再次设置事件。
    */
    event.events = EPOLLIN; //需要读取数据的事件
    //event.events = EPOLLIN | EPOLLET; // 使用边缘触发
    event.data.fd = serv_sock;
    epoll_ctl(epfd, EPOLL_CTL_ADD, serv_sock, &event); //例程epfd 中添加文件描述符 serv_sock，目的是监听 enevt 中的事件

    while (1)
    {
        // 最后一个参数，传递 -1 时，一直等待直到发生事件
        event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, -1); //获取改变了的文件描述符，返回改变的文件描述符的数量
        if (event_cnt == -1)
        {
            puts("epoll_wait() error");
            break;
        }

        for (i = 0; i < event_cnt; i++)
        {
            if (ep_events[i].data.fd == serv_sock) //客户端请求连接时
            {
                adr_sz = sizeof(clnt_adr);
                clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &adr_sz);
                event.events = EPOLLIN;
                event.data.fd = clnt_sock; //把客户端套接字添加进去
                epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sock, &event);
                printf("connected client : %d \n", clnt_sock);
            }
            else //是客户端套接字时
            {
                // 如果是边缘触发，就必须用while循环一次性读取所有数据。参考 echo_EPETserv.c
                str_len = read(ep_events[i].data.fd, buf, BUF_SIZE);
                if (str_len == 0)
                {
                    epoll_ctl(epfd, EPOLL_CTL_DEL, ep_events[i].data.fd, NULL); //从epoll中删除套接字，删除不需要填 event，填 NULL 就可以了
                    close(ep_events[i].data.fd);
                    printf("closed client : %d \n", ep_events[i].data.fd);
                }
                else
                {
                    write(ep_events[i].data.fd, buf, str_len);
                }
            }
        }
    }
    close(serv_sock);
    close(epfd);

    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

/**
 * @brief int epoll_create(int size);
 * 成功时返回 epoll 的文件描述符，失败时返回 -1
 * size：epoll 实例的大小
 * 调用 epoll_create 函数时创建的文件描述符保存空间称为「epoll 例程」，但有些情况下名称不同，
 * 需要稍加注意。通过参数 size 传递的值决定 epoll 例程的大小，
 * 但该值只是向操作系统提出的建议。换言之，size 并不用来决定 epoll 的大小，而仅供操作系统参考。
 */

/**
 * @brief int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
 * 成功时返回 0 ，失败时返回 -1
 * epfd：用于注册监视对象的 epoll 例程的文件描述符
 * op：用于制定监视对象的添加、删除或更改等操作
 * fd：需要注册的监视对象文件描述符
 * event：监视对象的事件类型
 * 
 * 下面是第二个参数的含义：
    - EPOLL_CTL_ADD：将文件描述符注册到 epoll 例程
    - EPOLL_CTL_DEL：从 epoll 例程中删除文件描述符
    - EPOLL_CTL_MOD：更改注册的文件描述符的关注事件发生情况
 */

/**
 * @brief int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
 * 成功时返回发生事件的文件描述符，失败时返回 -1
 * epfd : 表示事件发生监视范围的 epoll 例程的文件描述符
 * events : 保存发生事件的文件描述符集合的结构体地址值
 * maxevents : 第二个参数中可以保存的最大事件数
 * timeout : 以 1/1000 秒为单位的等待时间，传递 -1 传递 -1 时，一直等待直到发生事件
 */
