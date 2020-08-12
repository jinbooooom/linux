#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define strSize 1000
#define arrSize 100

char *arr2str(int *arr, int len, char *str)
{
	char stmp[20] = { "\0" };	// 用来存放单个数字转换后的字符串
    
	for (int i = 0; i < len; ++i)
	{
        sprintf(stmp, "%d", arr[i]);
		strcat(str, stmp);// 将 arr[i] 转换成字符串，拼接到 str 后面
        strcat(str, " ");
	}
	return str;
}


int main(int argc, char **argv)
{
    int len = 3;
    int arr[3] = {123, 456, 888};
    char str[20] = {'\0'};
    //sprintf(str, "%d", arr[1]);
    //printf("arr[0] = %d, itoa:%s",arr[0], str);
    arr2str(arr, len, str);
    printf("%s: ", str);

}