/*
 * @Author: jinboom
 * @Date: 2023-04-22 17:04:55
 * @LastEditors: jinboom
 * @LastEditTime: 2023-04-25 23:56:16
 * @FilePath: /linux/utils/memory/memoryPool/mpAlloc.cpp
 * @Description:
 *
 */

#include "mpAlloc.h"
#include <cassert>

FixedAlloc::FixedAlloc(const size_t count, const size_t size)
    : mCount(count), mBlockSize(size), mBaseAddress(nullptr), mEndAddress(nullptr), mMemHeaderSize(1)
{
    if (count <= 0 || size <= 0)
    {
        loge("invalid param, create alloc failed, count = %lu, size = %lu", count, size);
        exit(-1);
    }

    mTotalSize   = count * (size + mMemHeaderSize);
    mBaseAddress = new (std::nothrow) char[mTotalSize];
    logd("mTotalSize = %lu, count = %lu, size = %lu", mTotalSize, count, size);
    if (nullptr == mBaseAddress)
    {
        loge("alloc memory failed, count = %lu, size = %lu", count, size);
        exit(-1);
    }
    mEndAddress = mBaseAddress + mTotalSize;
    memset(mBaseAddress, 0, mTotalSize);
}

char *FixedAlloc::Malloc()
{
    size_t unitSize = mBlockSize + mMemHeaderSize;
    char *pHeader   = mBaseAddress;
    for (; pHeader < mEndAddress; pHeader += unitSize)
    {
        if (!(*pHeader))
        {
            *pHeader = (char)1;
            return pHeader + mMemHeaderSize;
        }
    }

    // TODO: 这里可以增加扩容，然后再分配，或者从其他规格的内存分配器中分配
    loge("current memory allocator(%lu) out of memory", mBlockSize);
    return nullptr;
}

error_t FixedAlloc::Free(char *vp)
{
    char *p = (char *)vp;
    if (isInvalidPointer(p))
    {
        loge("free invalid address %p", p);
        return -1;
    }

    *(vp - 1) = 0x0;

    return 0;
}

bool FixedAlloc::isInvalidPointer(char *vp)
{
    bool ok = vp > mBaseAddress && vp < mEndAddress && 0x1 == *(vp - 1) &&
              !((vp - mBaseAddress - mMemHeaderSize) % mBlockSize);
    return !ok;
}

DynamicAlloc::DynamicAlloc(const size_t size)
    : mBaseAddress(nullptr), mEndAddress(nullptr), mMemHeaderSize(4), mMemTailSize(12)
{
    if (size <= 0)
    {
        loge("invalid param, create alloc failed, size = %lu", size);
        exit(-1);
    }

    mTotalSize   = size + mMemHeaderSize + mMemTailSize;
    mBaseAddress = new (std::nothrow) char[mTotalSize];
    logd("mTotalSize = %lu, size = %lu", mTotalSize, size);
    if (nullptr == mBaseAddress)
    {
        loge("alloc memory failed, size = %lu", size);
        exit(-1);
    }
    mNextFitAddress = mBaseAddress;
    mEndAddress     = mBaseAddress + mTotalSize - mMemTailSize;
    memset(mBaseAddress, 0, mTotalSize);
    // setHeader(mBaseAddress + mMemHeaderSize, mTotalSize - mMemHeaderSize);
}

// used 为 0 说明空闲内存大小为 currentSize
// used = 0，currentSize = 0，说明是不能用的内存碎片
// used = 1，currentSize = 0，说明是 tail
char *DynamicAlloc::Malloc(uint32_t size)
{
    uint32_t realMallocSize = mMemHeaderSize + size + mMemHeaderSize;
    uint32_t currentSize    = 0;
    char used               = 0x1;
    for (char *p = mNextFitAddress; p < mEndAddress; p = p + currentSize + mMemHeaderSize * 2)
    {
        used        = *(p + mMemHeaderSize - 1);
        currentSize = ((*(uint32_t *)p) << 8);

        if (0 == used)
        {
            if (currentSize == size)
            {
                setHeader(p + mMemHeaderSize, size, 1);
            }
            else if (currentSize >= realMallocSize)
            {
                setHeader(p + mMemHeaderSize, size, 1);
                setHeader(p + realMallocSize, currentSize - realMallocSize, 0);
            }

            mNextFitAddress = p;
            return p + mMemHeaderSize;
        }
    }

    for (char *p = mBaseAddress; p < mNextFitAddress; p += currentSize)
    {
        // TODO
        break;
    }

    // TODO: 这里可以增加扩容，然后再分配
    loge("current memory allocator out of memory");
    return nullptr;
}

error_t DynamicAlloc::Free(char *vp)
{
    if (isInvalidPointer(vp))
    {
        loge("free invalid address %p", vp);
        return -1;
    }

    char used            = *(vp - 1);
    uint32_t currentSize = ((*(uint32_t *)(vp - mMemHeaderSize)) << 8);
    if (used)
    {
        setHeader(vp, currentSize, 0);
    }

    char *tvp              = (char *)vp;
    uint32_t tsize         = 0;
    uint32_t freeTotalSize = currentSize;

    // 向后遍历，合并空闲块
    do
    {
        tvp   = vp + freeTotalSize + mMemHeaderSize * 2;
        tsize = ((*(uint32_t *)(tvp - mMemHeaderSize)) << 8);
        used  = *(tvp - 1);

        if (0 == used)
        {
            freeTotalSize += tsize;
            setHeader(tvp, freeTotalSize, 0);
        }
        else
        {
            break;
        }
    } while (0 == used && tvp < mEndAddress);

    // 向前遍历，合并空闲块
    do
    {
        char *pPrevTail = vp - mMemHeaderSize * 2;
        tsize           = ((*(uint32_t *)(pPrevTail)) << 8);
        used            = *(pPrevTail + mMemHeaderSize - 1);
        tvp             = pPrevTail - tsize;

        if (0 == used)
        {
            freeTotalSize += tsize;
            setHeader(tvp, freeTotalSize, 0);
        }
        else
        {
            break;
        }

    } while (0 == used && tvp > mBaseAddress);

    return 0;
}

void DynamicAlloc::setHeader(char *vp, uint32_t size, uint8_t used)
{
    assert(size < 0xffffff);
    *(uint32_t *)(vp - mMemHeaderSize) = size;
    *(uint32_t *)(vp + size)           = size;
    *(vp - 1)                          = used;
    *(vp + size + mMemHeaderSize - 1)  = used;
}

bool DynamicAlloc::isInvalidPointer(char *vp)
{
    bool ok = vp > mBaseAddress && vp < mEndAddress && 0x1 == *(vp - 1);
    return !ok;
}
