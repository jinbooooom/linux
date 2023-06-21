#include <stdio.h>

int main()
{
    printf("commit sha1: %s\n", GIT_COMMIT_SHA1);
    printf("build time: %s, %s\n", __DATE__, __TIME__);
    return 0;
}
