/*
一个 UDP 客户端的例子，配合代码 udpServer.c 阅读
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <unistd.h> 
#include <errno.h>

#define MAX_BUF_SIZE 1024

void udpc_requ(int sockfd, const struct sockaddr_in *addr, int len)
{
    char buffer[MAX_BUF_SIZE];
    int n;
    while (1)
    {
        // 从键盘读数据，写到服务端
        fgets(buffer, MAX_BUF_SIZE, stdin);
        sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)addr, len);
        printf("I have sent to server %s", buffer);
        printf("Waiting respond from server\n");
        bzero(buffer, MAX_BUF_SIZE);
        // 从网络上读数据，写到屏幕
        n = recvfrom(sockfd, buffer, MAX_BUF_SIZE, 0, NULL, NULL);
        buffer[n] = 0;
        printf("I have received from server ");
        fputs(buffer, stdout);
        printf("\n");
    }
}

int main(int argc, char **argv)
{
    int sockfd, port;
    struct sockaddr_in addr;
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s server_ip server_port\n", argv[0]);
        exit(1);
    }

    if ((port = atoi(argv[2])) < 0)
    {
        fprintf(stderr, "Usage: %s server_ip server_port\n", argv[0]);
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    // 填充服务端的信息
    bzero(&addr, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_aton(argv[1], &addr.sin_addr) < 0)
    {
        fprintf(stderr, "IP error: %s\n", strerror(errno));
        exit(1);
    }

    udpc_requ(sockfd, &addr, sizeof(struct sockaddr_in));
    close(sockfd);
    return 0;
}

/*
jinbo@fang:~/gitme/linux/programming/network-programming$ ./udpClient.o 127.0.0.1 8888
Hello
I have sent to server Hello
Waiting respond from server
I have received from server HELLO

world!
I have sent to server world!
Waiting respond from server
I have received from server WORLD!
*/