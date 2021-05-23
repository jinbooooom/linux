#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
int main(int argc, char *argv[])
{
	DIR *dp;
	struct dirent *entp;
	if (argc != 2) {
		printf("usage: showdir dirname\n");
		exit(0);
	}
	if ((dp = opendir(argv[1])) == NULL) {
		perror("opendir");
		exit(-1);
	}
	while ((entp = readdir(dp)) != NULL)
		printf("%s\n", entp->d_name);
	closedir(dp);
	return 0;
}
