
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int compare(int x, int y, char cmp)
{
	if (cmp == '0')	
	{
		if (x > y)
			return 1;
		else
			return 0;
	}
	else
	{
		if (x <= y) return 1;
		else return 0;
	}
}

void bubble(int *arr, int len, char cmp)
{
	int t;
	for (int i = 0; i < len - 1; ++i)  // 一共有 len - 1 轮
		for (int j = 0; j < len - i - 1; ++j)  // 每轮有 len - i - 1 次比较
			//if (arr[j] > arr[j + 1])  // 前面数字大于后面数字，就交换
			if (compare(arr[j], arr[j + 1], cmp))
				t = arr[j], arr[j] = arr[j + 1], arr[j + 1] = t;
}

void insert(int *arr, int len, char cmp)
{
	int i, j, t;
	for (i = 1; i < len; ++i)  // 从第二个数开始（索引1），一共 len - 1 轮
	{
		t = arr[i];
		//for (j = i - 1; j >= 0 && t < arr[j]; --j)
		for (j = i - 1; j >= 0 && compare(arr[j], t, cmp); --j)
			arr[j + 1] = arr[j];
		arr[j + 1] = t;
	}
}


void mergeCore(int *arr, int low, int mid, int high, char cmp)
{
	int i, k;
	// int *tmp = new int[high - low + 1];
	int *tmp = (int *)malloc((high - low + 1) * sizeof(int));
	int left_low = low, left_high = mid;
	int right_low = mid + 1, right_high = high;
	for (k = 0; left_low <= left_high && right_low <= right_high; k++)
		//if (arr[left_low] <= arr[right_low])
		if (compare(arr[right_low], arr[left_low], cmp))
			tmp[k] = arr[left_low++];
		else
			tmp[k] = arr[right_low++];
	if (left_low <= left_high)
		for (i = left_low; i <= left_high; i++)
			tmp[k++] = arr[i];
	if (right_low <= right_high)
		for (i = right_low; i <= right_high; i++)
			tmp[k++] = arr[i];
	for (i = 0; i < high - low + 1; i++)
		arr[low + i] = tmp[i];
	// delete[] tmp;
	free(tmp); 
}

void merge(int *arr, unsigned int first, unsigned int last, char cmp)
{
	int mid = 0;
	if (first < last)
	{
		mid = (first + last) / 2;
		merge(arr, first, mid, cmp);
		merge(arr, mid + 1, last, cmp);
		mergeCore(arr, first, mid, last, cmp);
	}
}

int partition(int *arr, int low, int high, char cmp)
{
	int key = arr[low];  // 默认第一个数作为基准
	while (low < high) 
	{
		//while (low < high && arr[high] >= key)
		while (low < high && compare(arr[high], key, cmp))
			high--;
		if (low < high)
			arr[low++] = arr[high];
		//while (low < high && arr[low] <= key)
		while (low < high && compare(key, arr[low], cmp))
			low++;
		if (low < high)
			arr[high--] = arr[low];
	}
	arr[low] = key;
	return low;
}

void quick(int *arr, int start, int end, char cmp)
{
	int pos;
	if (start < end)
	{
		pos = partition(arr, start, end, cmp);
		quick(arr, start, pos - 1, cmp);
		quick(arr, pos + 1, end, cmp);
	}
}

void my_select(int *arr, int len, char cmp)
{
	int i, j, k, t;
	for (i = 0; i < len - 1; ++i)
	{
		k = i;
		for (j = i + 1; j < len; j++)
			//if (arr[j] < arr[k])	// 把比 arr[k] 更小的值的索引赋给 k
			if (compare(arr[k], arr[j], cmp))
				k = j;
		if (k != i)
			t = arr[i], arr[i] = arr[k], arr[k] = t;
	}
}

/*
shell 排序是对插入排序的一个改进
*/
void shell(int *arr,int len, char cmp)
{
    int i, j, x, d;
    d = len/2;
    // 当增量 d 为 1 时，执行的就是插入排序。
    // 但实际上当 d = 1 时，此时数组很有序，只需要少量操作就可以排完序。
    while(d >= 1)
    {
        for(i = d; i < len; ++i)
        {
            x = arr[i];
            j = i - d;
            //while(j >= 0 && arr[j] > x)
			while(j >= 0 && compare(arr[j], x, cmp))
            {
                arr[j + d] = arr[j];
                j -= d;
            }
            arr[j + d] = x;
        }
        d /= 2;
    }
}

void sort(int *arr, int len, char *sort_name, char cmp)
{
	if (!strcmp(sort_name, "01")) 
	{
		printf("call insert sort...\n");
		insert(arr, len, cmp);				// insert	
	}			
	else if (!strcmp(sort_name, "02")) 
	{
		printf("call shell sort...\n");
		shell(arr, len, cmp);				// shell
	}
	else if (!strcmp(sort_name, "03")) 
	{
		printf("call select sort...\n");
		my_select(arr, len, cmp);			// select
	}
	/* else if (!strcmp(sort_name, "04")) 
	{
		heap(par);					// heap
	}
	*/
	else if (!strcmp(sort_name, "05")) 
	{
		printf("call bubble sort...\n");
		bubble(arr, len, cmp);				// bubble
	}
	else if (!strcmp(sort_name, "06")) 
	{
		printf("call quick sort...\n");
		quick(arr, 0, len - 1, cmp);		// quick
	}
	else if (!strcmp(sort_name, "07")) 
	{
		printf("call merge sort...\n");
		merge(arr, 0, len - 1, cmp);		// merge
	}
	else 
	{
		printf("call merge sort...\n");
		merge(arr, 0, len - 1, cmp);		// "00":其他情况均用归并排序
	}		
	cmp = '\0';					
}