/*
一个 TCP 服务器的例子，配合代码 tcpClient.c 阅读
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

char host_name[20];
int port = 8000;

int main(int argc, char **argv)
{
	struct sockaddr_in sin, pin;
	int sock_descriptor, temp_sock_descriptor, address_size;
	int i, len, on = 1;
	char buf[16384];

	sock_descriptor = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(sock_descriptor, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	bzero(&sin, sizeof(sin));  // 置字符串的前 n 位为 0，无返回值

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(port);
	if (bind(sock_descriptor, (struct sockaddr *)&sin, sizeof(sin)) == -1) // 返回 -1 为失败
	{
		perror("call to bind\n");
		exit(1);
	}
	if (listen(sock_descriptor, 100) == -1)
	{
		perror("call to listen\n");
		exit(1);
	}
	printf("Accepting connections...\n");

	while (1)
	{
		address_size = sizeof(pin);
		temp_sock_descriptor = accept(sock_descriptor, (struct sockaddr *)&pin, &address_size);
		if (temp_sock_descriptor == -1)
		{
			perror("call to accept\n");
			exit(1);
		}
		if (recv(temp_sock_descriptor, buf, 16384, 0) == -1) // 从 TCP 的另一端接收数据; 设置大小为 16384 的缓冲区 buf 用来接收收到的数据
		{
			perror("call to recv");
			exit(1);
		}
		inet_ntop(AF_INET, &pin.sin_addr, host_name, sizeof(host_name));
		printf("received from client(%s): %s\n", host_name, buf);

		len = strlen(buf);
		for (i = 0; i < len; ++i)
		{
			buf[i] = toupper(buf[i]);
		}

		if (send(temp_sock_descriptor, buf, len + 1, 0) == -1) // 向 TCP 另一端（客户端）发送数据
		{
			perror("call to send\n");
			exit(1);
		}
		close(temp_sock_descriptor); // 结束通信
	}
}

/*

jinbo@fang:~/gitme/linux/programming/network-programming$ ./tcpServer.o Accepting connections...
received from client(127.0.0.1): hello
received from client(127.0.0.1): world!

*/

