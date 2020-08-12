#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

int* str2arr(char *str, int *arr, int *cnt)
{
	// cnt：通过处理 str 计算 arr 的长度，存放在 cnt 中
	char stmp[20] = { '\0' };	// 临时提取的单个数字字符串
	int itmp = 0;   			// 字符数组 stmp 的索引
	for (int i = 0; i < STR_SIZE; ++i)
	{
		if (str[i] >= '0' && str[i] <= '9' || str[i] == '-' || str[i] == '+')
		{   
			stmp[itmp++] = str[i];
		}
		else
		{
			stmp[itmp] = '\0';
			itmp = 0;
			if (stmp[0])
			{
				arr[(*cnt)++] = atoi(stmp);
			}
		}  
	}
	show(arr, *cnt);
}

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


void show(int *arr, int len)
{
	printf("the data of array is： ");
	for (int i = 0; i < len; ++i)
	{
		printf("%d ", arr[i]);
	}
	printf("\n");
}
