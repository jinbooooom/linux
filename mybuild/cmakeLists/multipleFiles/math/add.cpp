#include "stdio.h"

int add(int x, int y)
{
    printf("开始计算 %d + %d, %s:%s:%d\n", x, y, __FILE__, __FUNCTION__, __LINE__);
    return x + y;
}