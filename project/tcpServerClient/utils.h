#ifndef _UTILS_H
#define _UTILS_H

#define STR_SIZE 10000
#define ARR_SIZE 100
#define HEAD_SIZE 4
#define BUF_SIZE 16384

char *getSortID(const char *sort_name);
char *getCmp(const char *cmp);
void packet(char *dst, const char *src, const char *sort_name, const char *cmp);
void unpack(const char *dst, char *sort_name, char *cmp);
void show(const int *arr, int len);
void stoa(const char *str, int *arr, int *len);
void atos(const int *arr, int len, char *str);

#endif