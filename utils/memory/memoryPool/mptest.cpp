/*
 * @Author: jinboom
 * @Date: 2023-04-22 18:42:51
 * @LastEditors: jinboom
 * @LastEditTime: 2023-04-24 21:34:21
 * @FilePath: /linux/mnt/d/gitme/linux/utils/memory/memoryPool/mptest.cpp
 * @Description: memory pool test
 *
 */

#include "memoryPool.h"

int main()
{
    MemoryPool mp(18, 2);


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
