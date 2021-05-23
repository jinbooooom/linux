#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
	int srcfd, destfd, total, num;
	char buf[4096];
	if ((srcfd = open(argv[1], O_RDONLY)) < 0)
		{ perror("open"); exit(-1); }
	if ((destfd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0)
		{ perror("open"); exit(-2); }
	while ((total = read(srcfd, buf, 4096)) > 0) {
		while (((num = write(destfd, buf, total)) < total)  && (num >=0))
			total -= num;
		if (num < 0) {
			printf("write error\n");
			exit(-2);
		}
	}
	if (total < 0)
		printf("read error\n");
	close(srcfd);
	close(destfd);
}
