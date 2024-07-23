/*
功能：将 unsigned int 数据的主机序列，转换成网络字节序列。
解析：
整数的低位放在内存的低地址，高位放在内存的高地址，这样的系统被称为小端系统，与之相反的是大端系统。
对于 0x3132 在小端系统的存放如下所示：

高地址-----------------低地址
0011	0001	0011	0010

在大端系统中，存放如下所示：
高地址-----------------低地址
0011	0010	0011	0001

一般情况下：

 发送机-----------》网络----------------》接收机
（小端）    （大端，网络字节序）		 （大端）

*/

#include <stdio.h>

int main(int argc, char **argv)
{
	unsigned num = 0xab127980;
	unsigned char *pc;

	printf("nums address is %p, and it's value is 0x %x \n\n", &num, num);
	pc = (unsigned char *)&num;
	for (size_t i = 0; i < 4; ++i)
	{
		printf("%p: 0x%x\n", pc, (unsigned int)*pc);
		++pc;
	}

	unsigned short port;
	port = 0x6789;
	printf("port number in   host  byteorder is 0x%x\n", port);
	printf("port number in network byteorder is 0x%x\n", htons(port));
}
