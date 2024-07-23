/*
一个 TCP 迭代服务器的例子，处理客户端发送过来的数据，排好序后发送给客户端
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

#include "sort.h"
#include "utils.h"

char host_name[20];
int port = 8888;

int main(int argc, char **argv)
{
	struct sockaddr_in sin, pin;	// sockaddr_in: IPv4 地址，sockaddr_in6: IPv6 地址 
	int sock_descriptor, temp_sock_descriptor, address_size;
	int i, len, on = 1;
	char buf[BUF_SIZE];

	char sort_name = '\0';
	char cmp = '0';

	// 创建一个用于网络通信的套接字，并返回该套接字的整数描述符
	sock_descriptor = socket(AF_INET, SOCK_STREAM, 0);
	// 四次挥手时服务器处于 TIME_WAIT 状态，需要等待 2MSL 时间。
	// 此时监听服务器试图绑定到该端口，操作系统不会允许，可以在 socket 和 bind 之间设置允许地址重用套接口选项
	setsockopt(sock_descriptor, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	bzero(&sin, sizeof(sin));			// 置字符串的前 n 位为 0，无返回值
	// 填写本机 socket 地址结构体
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;	// 转换过来就是0.0.0.0，表示本机的所有IP，因为有些机子不止一块网卡
	sin.sin_port = htons(port);			// 调用 htons 对端口号进行字节序转换
	// 将 socket 与 IP 和端口号绑定，组成三元组
	if (bind(sock_descriptor, (struct sockaddr *)&sin, sizeof(sin)) == -1) // 返回 -1 为失败
	{
		perror("call to bind\n");
		exit(1);
	}
	// 将 socket 转换为被动（监听）套接口，用于接收客户端的连接请求，连接请求的队列大小设置为 100
	if (listen(sock_descriptor, 100) == -1)
	{
		perror("call to listen\n");
		exit(1);
	}
	printf("Accepting connections...\n");

	int arr[ARR_SIZE] = { 0 };
	int size = 0;	// 数组的长度
	while (1)
	{
		address_size = sizeof(pin);
		// 获取到了客户端的连接，使 temp_sock_descriptor 为一个完整的 5 元组，因此可通过该套接口收发数据
		temp_sock_descriptor = accept(sock_descriptor, (struct sockaddr *)&pin, &address_size);
		if (temp_sock_descriptor == -1)
		{
			perror("call to accept\n"); 
			exit(1);
		}
		// 从 TCP 的另一端接收数据; 设置大小为 BUF_SIZE 的缓冲区 buf 用来接收收到的数据
		if (recv(temp_sock_descriptor, buf, BUF_SIZE, 0) == -1)
		{
			perror("call to recv");
			exit(1);
		}
		// 将二进制整数 －> 点分十进制
		inet_ntop(AF_INET, &pin.sin_addr, host_name, sizeof(host_name));
		printf("received data from client(%s): %s\n", host_name, buf + HEAD_SIZE);

		len = strlen(buf);
		memset(arr, 0, sizeof(int) * ARR_SIZE);
		size = 0;
		stoa(buf + HEAD_SIZE, arr, &size);	// 前面 HEAD_SIZE 个数据为包头，后面的才是真正的数据
		unpack(buf, &sort_name, &cmp);
		// printf("buf:%s\nsort_name:%s\ncmp:%c\n", buf, sort_name, cmp);
		sort(arr, size, sort_name, cmp);
		// show(arr, size);
		// printf("\n%s\n", buf);
		
		memset(buf, '\0', sizeof(buf));
		atos(arr, size, buf);
		printf("after sort：%s\n\n", buf);

		if (send(temp_sock_descriptor, buf, len + 1, 0) == -1) // 向 TCP 另一端（客户端）发送数据
		{
			perror("call to send\n");
			exit(1);
		}
		// sleep(5);
		close(temp_sock_descriptor); // 结束通信
	}
}


