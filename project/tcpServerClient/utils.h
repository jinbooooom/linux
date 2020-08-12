#ifndef UTILS_H
#define UTILS_H

#define STR_SIZE 1000
#define ARR_SIZE 100

void show(int *arr, int len);
int* str2arr(char *str, int *arr, int *len);
char *arr2str(int *arr, int len, char *str);

#endif