/*
一个 UDP 服务器的例子，配合代码 udpClient.c 阅读
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

#define SERVER_PORT 8888
#define MAX_MSG_SIZE 1024

void udps_respon(int sockfd)
{
    struct sockaddr_in addr;
    int n, i;
    unsigned int addrlen;
    char msg[MAX_MSG_SIZE];
    while (1)
    {
        n = recvfrom(sockfd, msg, MAX_MSG_SIZE, 0, (struct sockaddr *)&addr, &addrlen);
        msg[n] = 0;
        fprintf(stdout, "I have received %s", msg);

        n = strlen(msg);
        for (i = 0; i < n; ++i)
            msg[i] = toupper(msg[i]);
        sendto(sockfd, msg, n, 0, (struct sockaddr *)&addr, addrlen);
    }
}

int main(int argc, char **argv)
{
	
	int sockfd;
	struct sockaddr_in addr;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	bzero(&addr, sizeof(struct sockaddr_in));  // 置字符串的前 n 位为 0，无返回值

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(SERVER_PORT);
	if (bind(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0)
	{
		perror("bind\n");
		exit(1);
	}
	
    udps_respon(sockfd);
	close(sockfd); // 结束通信
	return 0;
}

/*
jinbo@fang:~/gitme/linux/programming/network-programming$ ./udpServer.o 
I have received Hello
I have received world!
*/

