/*
 * @Author: jinboom
 * @Date: 2023-04-22 17:04:55
 * @LastEditors: jinboom
 * @LastEditTime: 2023-04-22 19:54:38
 * @FilePath: /linux/utils/memory/memoryPool/mpAlloc.cpp
 * @Description:
 *
 */

#include "mpAlloc.h"

MpAlloc::MpAlloc(const size_t count, const size_t size)
    : mCount(count), mBlockSize(size), mBaseAddress(nullptr), mEndAddress(nullptr)
{
    if (count <= 0 || size <= 0)
    {
        loge("invalid param, create alloc failed, count = %lu, size = %lu", count, size);
        exit(-1);
    }

    mTotalSize   = count * (size + MEM_HEADER_SIZE);
    mBaseAddress = new (std::nothrow) char[size];
    if (nullptr == mBaseAddress)
    {
        loge("alloc memory failed, count = %lu, size = %lu", count, size);
        exit(-1);
    }
    mEndAddress = mBaseAddress + mTotalSize;

    memset(mBaseAddress, 0, mTotalSize);
}

char *MpAlloc::Malloc()
{
    size_t unitSize = mBlockSize + MEM_HEADER_SIZE;
    char *pHeader   = mBaseAddress;
    for (; pHeader < mEndAddress; pHeader += unitSize)
    {
        if (!(*pHeader))
        {
            *pHeader = (char)1;
            return pHeader + MEM_HEADER_SIZE;
        }
    }

    // TODO: 这里可以增加扩容，然后再分配，或者从其他规格的内存分配器中分配
    loge("current memory allocator(%lu) out of memory", mBlockSize);
    return nullptr;
}

error_t MpAlloc::Free(char *vp)
{
    char *p = (char *)vp;
    if (isInvalidPointer(p))
    {
        loge("free invalid address %p", p);
        return -1;
    }

    *(p - 1) = 0;

    return 0;
}

bool MpAlloc::isInvalidPointer(char *vp)
{
    return !(vp > mBaseAddress && vp < mEndAddress);
}
