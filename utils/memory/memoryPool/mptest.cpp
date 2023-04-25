/*
 * @Author: jinboom
 * @Date: 2023-04-22 18:42:51
 * @LastEditors: jinboom
 * @LastEditTime: 2023-04-25 23:57:00
 * @FilePath: /linux/utils/memory/memoryPool/mptest.cpp
 * @Description: memory pool test
 *
 */

#include "memoryPool.h"

#include <cassert>
#include <map>
#include <unordered_map>

void header(char *p, uint32_t size)
{
    *(uint32_t *)p = size;
    *(p + 3)       = 0x1;
}

void resetheader(char *p)
{
    *(p + 3) = 0x0;
}

int main()
{
    int a   = 0x0;
    char *p = (char *)&a;
    char *q = p + 1;
    char *r = q + 1;
    char *s = r + 1;
    *p      = 0x1;
    *q      = 0x2;
    *r      = 0x3;
    *s      = 0x4;

    printf("%p %p, %x\n", p, q, *(int *)p);  // 0x04030201
    header(p, 5);
    printf("%x\n", *(uint32_t *)p);
    resetheader(p);
    printf("%x\n", *(uint32_t *)p);

    // assert(!(2 >> 1));

    // return 0;

    // MemoryPool mp(2, 17);
    MemoryPool mp(128);

    char *p1 = mp.Malloc(3);
    char *p2 = mp.Malloc(7);
    char *p3 = mp.Malloc(15);
    char *p4 = mp.Malloc(17);

    logi("p1 = %p", p1);
    logi("p2 = %p", p2);
    logi("p3 = %p", p3);
    logi("p4 = %p", p4);

    mp.Free(p3);
    p3 = nullptr;
    p3 = mp.Malloc(15);
    logi("p3 = %p", p3);

    return 0;
}
