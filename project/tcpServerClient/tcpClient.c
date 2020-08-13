/*
一个 TCP 客户端的例子，配合代码 tcpServer.c 阅读
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

char *host_name = "127.0.0.1";	// 这是主机的地址，自己与自己通信，方便调试
int port = 8000;

// 命令行：可执行文件名 本机IP 输入数字字符串（不要用空格隔开，其它的都可以）
int main(int argc, char **argv)
{
	char buf[8192];
	int socket_descriptor;
	struct sockaddr_in pin;
	char *str = "9,8,5,15,6";
	if (argc < 2)
	{
		printf("Usage:client Any test string ip address\n");
		printf("We will send default test numbers.\n");
	}
	else
	{
		str = argv[2];
		if (argc == 3)
			host_name = argv[1];
	}
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
	if (write(socket_descriptor, str, strlen(str) + 1) == -1)
	{
		perror("Error in send\n");
		exit(1);
	}
	printf("..sent message.. wait for response..\n");

	if (read(socket_descriptor, buf, 8192) == -1)
	{
		perror("Error in receiving response from server\n");
		exit(1);
	}
	printf("\nResponse from server:\n\n%s\n", buf);
	close(socket_descriptor);
	return 1;
}




