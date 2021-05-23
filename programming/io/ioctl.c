#include <linux/cdrom.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int main(void)
{
	int fd = open("/dev/cdrom", O_RDONLY | O_NONBLOCK);
	if (fd < 0) {
		perror("open");
		exit(-1);
	}

//	ioctl(fd, CDROM_LOCKDOOR, 0);
	if (!ioctl(fd, CDROMEJECT, NULL)) {
		printf("eject cdrom succeed\n");
	} else {
		printf("eject cdrom failed\n");
	}
/*
	sleep(2);
	ioctl(fd, CDROM_LOCKDOOR, 0);
	if (!ioctl(fd, CDROMCLOSETRAY, NULL)) {
		printf("close cdrom succeed\n");
	} else {
		printf("close cdrom failed\n");
	}
*/
	return 0;
}
