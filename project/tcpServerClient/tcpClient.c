/*
一个 TCP 客户端，通过命令行输入数据，服务端排好序后，返回给客户端
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>

#include "utils.h"

char *host_name = "127.0.0.1";	// 这是主机的地址，自己与自己通信，方便调试
int port = 8000;

// 命令行：可执行文件名 本机IP 输入数字字符串（不要用空格隔开，其它的都可以） 排序算法名称 升序/降序
int main(int argc, char **argv)
{
	char buf[8192];
	//char data[8192];
	int socket_descriptor;
	struct sockaddr_in pin;
	char *str = "9,8,5,15,6";
	char *sort_name = "merge";
	char *cmp = "ASC";	// ASC，DES；
	if (argc < 3)	// 用户至少要输入 3 个参数
	{
		printf("too few parameters.\n");
		printf("We will send default test numbers.\n");
	}
	else
	{
		str = argv[2];
		host_name = argv[1];
		if (argc >= 4) 
			sort_name = argv[3];
		else 
			sort_name = "merge";
		if (argc >= 5)      // 说明 5 个参数用户都输入了
			cmp = argv[4];
		else
		{
			cmp = "ASC";    // 默认升序
		}
	}
    printf("data:                   %s\nsort algorithm:         %s\nasceding or descending: %s\n", str, sort_name, cmp);
    packet(buf, str, sort_name, cmp);
	// printf("after packet:%s\n", data);
	bzero(&pin, sizeof(pin));

	pin.sin_family = AF_INET;
	inet_pton(AF_INET, host_name, &pin.sin_addr);	// 将点分十进制 －> 二进制整数
	pin.sin_port = htons(port);
	if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("Error opening socket\n");
		exit(1);
	}

	if (connect(socket_descriptor, (struct sockaddr *)&pin, sizeof(pin)) == -1) // 三次握手
	{
		perror("Error connecting to socket\n");
		exit(1);
	}

	printf("Sending message %s to server...\n", str);
	if (write(socket_descriptor, buf, strlen(buf) + 1) == -1)
	{
		perror("Error in send\n");
		exit(1);
	}
	printf("..sent message.. wait for response..\n");
	// 之前用 buf 存放加包头的数据，现在用 buf 存从服务端发送过来的数据，故要将 buf 全置零
	memset(buf, '\0', sizeof(buf)); 
	if (read(socket_descriptor, buf, 8192) == -1)
	{
		perror("Error in receiving response from server\n");
		exit(1);
	}
	printf("\nResponse from server:\n\n%s\n", buf);
	close(socket_descriptor);
	return 1;
}




