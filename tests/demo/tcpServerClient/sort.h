#ifndef _SORT_H
#define _SORT_H

/*
void bubble(int *arr, int len, char cmp);
void bubble(int *arr, int len);
void insert(int *arr, int len);
void merge(int *arr, unsigned int first, unsigned int last);
void quick(int *arr, int start, int end);
void my_select(int *arr, int len);  // 跟系统函数重名了
void shell(int *arr,int len);
*/

void sort(int *arr, int len, char sort_name, char cmp);

#endif
