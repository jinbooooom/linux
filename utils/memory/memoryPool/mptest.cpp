/*
 * @Author: jinboom
 * @Date: 2023-04-22 18:42:51
 * @LastEditors: jinboom
 * @LastEditTime: 2023-04-22 20:00:20
 * @FilePath: /linux/utils/memory/memoryPool/mptest.cpp
 * @Description: memory pool test
 *
 */

#include "mpAlloc.h"

int main()
{
    MpAlloc mp(2, 4);
    char *p1 = mp.Malloc();
    char *p2 = mp.Malloc();
    char *p3 = mp.Malloc();

    logi("base = %p, end = %p", mp.mBaseAddress, mp.mEndAddress);
    logi("p1 = %p", p1);
    logi("p2 = %p", p2);
    logi("p3 = %p", p3);

    mp.Free(p1);
    p1 = nullptr;
    p3 = mp.Malloc();
    logi("p3 = %p", p3);

    return 0;
}
