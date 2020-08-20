#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "sort.h"

/*************************************************************
 * 
 * 将客户端输入的排序算法的名称转换为字符序列
 * 
**************************************************************/
char *getSortID(const char *sort_name)
{
	if (!strcmp(sort_name, "insert")) return "1";
	else if (!strcmp(sort_name, "shell")) return "2";
	else if (!strcmp(sort_name, "select")) return "3";
	// else if (!strcmp(sort_name, "heap")) return "4";
	else if (!strcmp(sort_name, "bubble")) return "5";
	else if (!strcmp(sort_name, "quick")) return "6";
	else if (!strcmp(sort_name, "merge")) return "7";
	else return "\0";
}

/*************************************************************
 * 
 * 将客户端输入的排序方式（升序还是降序）转换为字符序列
 * 
**************************************************************/
char *getCmp(const char *cmp)
{
	if (!strcmp(cmp, "DES") || !strcmp(cmp, "des") || !strcmp(cmp, "-")) 
		return "1";	// 降序，从大到小
	else 
		return "0";	// 升序，从小到大
}

/*************************************************************
 * 将要发送给服务端排序的数字字符串 src 前面加一个 HEAD_SIZE 大小的包头
 * 包头每一个字节的内容如下：
 * 1：选用何种排序算法
 * 2：升序还是降序
 * 3~HEAD_SIZE:预留项
**************************************************************/
void packet(char *dst, const char *src, const char *sort_name, const char *cmp)
{
	char *sortID = getSortID(sort_name);
	strcat(dst, sortID);
	strcat(dst, getCmp(cmp));
    int fill_size = HEAD_SIZE - (int)strlen(dst);
	char *fill = (char *)malloc(fill_size > 0 ? fill_size : 0);
	memset(fill, ' ', fill_size);
	strcat(dst, fill);
	strcat(dst, src);
	free(fill);
}

/*************************************************************
 * 服务端从客户端打包的字节流数据 dst 中，把包头里的信息解析出来
 * sort_name:调用何种排序算法
 * cmp：升序还是降序
**************************************************************/
void unpack(const char *dst, char *sort_name, char *cmp)
{
	/*
	*sort_name = *dst;
	*(sort_name + 1) = *(dst + 1);
	*cmp = *(dst + 2);
	*/
	*sort_name = *dst;
	*cmp = *(dst + 1);
}

/*************************************************************
 * 
 * 将包含一系列数字的字符串 str 转换成整数数组 arr
 * 
**************************************************************/
void stoa(const char *str, int *arr, int *cnt) 
{
	// cnt：通过处理 str 计算 arr 的长度，存放在 cnt 中
	char stmp[20] = { '\0' };	// 临时提取的单个数字字符串
	int itmp = 0;   			// 字符数组 stmp 的索引
	for (int i = 0; i < strlen(str) + 1; ++i)
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

/*************************************************************
 * 
 * 将整数数组 arr 转换成字节序 str
 * 
**************************************************************/
void atos(const int *arr, int len, char *str)	// 
{
	char stmp[20] = { "\0" };	// 用来存放单个数字转换后的字符串

	for (int i = 0; i < len; ++i)
	{
        sprintf(stmp, "%d", arr[i]);
		strcat(str, stmp);// 将 arr[i] 转换成字符串，拼接到 str 后面
        strcat(str, " ");
	}
}

/*************************************************************
 * 
 * 打印数组 arr
 * 
**************************************************************/
void show(const int *arr, int len)
{
	printf("the data of array is： ");
	for (int i = 0; i < len; ++i)
	{
		printf("%d ", arr[i]);
	}
	printf("\n");
}
