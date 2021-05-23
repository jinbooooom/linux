#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
int main(void)
{
	int fd, flag;
	if ((fd = open("/tmp/yangzhu.txt", O_RDWR | O_APPEND | O_CREAT | O_TRUNC, 00666)) < 0) {
		perror("open");
		exit(-1);
	}
	write(fd, "yang", 4);
	lseek(fd, 0, SEEK_SET);
/*
	flag = fcntl(fd, F_GETFL);
	flag = flag & ~O_APPEND;
	fcntl(fd, F_SETFL, flag);
*/
	write(fd, "zhu", 3);
	return 0;
}

	
