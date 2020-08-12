
#include <stdlib.h>

void bubble(int *arr, int len)
{
	int t;
	for (int i = 0; i < len - 1; ++i)  // 一共有 len - 1 轮
		for (int j = 0; j < len - i - 1; ++j)  // 每轮有 len - i - 1 次比较
			if (arr[j] > arr[j + 1])  // 前面数字大于后面数字，就交换
				t = arr[j], arr[j] = arr[j + 1], arr[j + 1] = t;
}

void insert(int *arr, int len)
{
	int i, j, t;
	for (i = 1; i < len; ++i)  // 从第二个数开始（索引1），一共 len - 1 轮
	{
		t = arr[i];
		for (j = i - 1; j >= 0 && t < arr[j]; --j)
			arr[j + 1] = arr[j];
		arr[j + 1] = t;
	}
}


void mergeCore(int *arr, int low, int mid, int high)
{
	int i, k;
	// int *tmp = new int[high - low + 1];
	int *tmp = (int *)malloc((high - low + 1) * sizeof(int));
	int left_low = low, left_high = mid;
	int right_low = mid + 1, right_high = high;
	for (k = 0; left_low <= left_high && right_low <= right_high; k++)
		if (arr[left_low] <= arr[right_low])
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

void merge(int *arr, unsigned int first, unsigned int last)
{
	int mid = 0;
	if (first < last)
	{
		mid = (first + last) / 2;
		merge(arr, first, mid);
		merge(arr, mid + 1, last);
		mergeCore(arr, first, mid, last);
	}
}

int partition(int *arr, int low, int high)
{
	int key = arr[low];  // 默认第一个数作为基准
	while (low < high) 
	{
		while (low < high && arr[high] >= key)
			high--;
		if (low < high)
			arr[low++] = arr[high];
		while (low < high && arr[low] <= key)
			low++;
		if (low < high)
			arr[high--] = arr[low];
	}
	arr[low] = key;
	return low;
}

void quick(int *arr, int start, int end)
{
	int pos;
	if (start < end)
	{
		pos = partition(arr, start, end);
		quick(arr, start, pos - 1);
		quick(arr, pos + 1, end);
	}
}

void my_select(int *arr, int len)
{
	int i, j, k, t;
	for (i = 0; i < len - 1; ++i)
	{
		k = i;
		for (j = i + 1; j < len; j++)
			if (arr[j] < arr[k])	// 把比 arr[k] 更小的值的索引赋给 k
				k = j;
		if (k != i)
			t = arr[i], arr[i] = arr[k], arr[k] = t;
	}
}

/*
shell 排序是对插入排序的一个改进
*/
void shell(int *arr,int len)
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
            while(j >= 0 && arr[j] > x)
            {
                arr[j + d] = arr[j];
                j -= d;
            }
            arr[j + d] = x;
        }
        d /= 2;
    }
}