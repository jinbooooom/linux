/*
 * @Author: jinboom
 * @Date: 2023-04-22 20:03:35
 * @LastEditors: jinboom
 * @LastEditTime: 2023-04-24 21:36:06
 * @FilePath: /linux/utils/memory/memoryPool/memoryPool.cpp
 * @Description:
 *
 */

#include "memoryPool.h"

std::map<size_t, size_t> MemoryPool::sDefaultMemCfg = {
    {MEM_4B, 4},
    {MEM_8B, 2},
    {MEM_16B, 1},
};

MemoryPool::MemoryPool(/* args */)
{
    size_t size  = 0;
    size_t count = 0;
    for (auto it = sDefaultMemCfg.begin(); it != sDefaultMemCfg.end(); ++it)  // 4B, 8B, 16B
    {
        size  = it->first;
        count = it->second;
        if (createAllocator(count, size))  // TODO: count 根据 size 动态调整
        {
            loge("create allocator(%lu) failed", size);
            exit(-1);
        }
    }
}

MemoryPool::MemoryPool(const size_t count, const size_t size): MemoryPool()
{
    if (count <= 0 || size <= 0)
    {
        loge("invalid param, create memory pool failed, count = %lu, size = %lu", count, size);
        exit(-1);
    }

    if (createAllocator(count, size))
    {
        loge("create allocator(%lu) failed", size);
        exit(-1);
    }
}

MemoryPool::~MemoryPool()
{
    for (auto it = mAllocators.begin(); it != mAllocators.end(); ++it)
    {
        if (it->second)
        {
            delete it->second;
            it->second = nullptr;
        }
    }
}

char *MemoryPool::Malloc(size_t size)
{
    char *p          = nullptr;
    size_t blockSize = 0;
    for (auto it = sDefaultMemCfg.begin(); it != sDefaultMemCfg.end(); ++it)
    {
        blockSize = it->first;
        if (size <= blockSize)
        {
            p = mAllocators[blockSize]->Malloc();
            if (p)
            {
                return p;
            }
        }
    }

    p = (char *)malloc(size);
    return p;
}

error_t MemoryPool::Free(char *vp)
{
    // TODO：
    MpAlloc *allocator = nullptr;
    error_t ret        = getAllocator(vp, &allocator);
    if (ret)
    {
        return allocator->Free(vp);
    }
    else
    {
        free(vp);
    }

    return 0;
}

error_t MemoryPool::createAllocator(const size_t count, const size_t size)
{
    if (count <= 0 || size <= 0)
    {
        loge("invalid param, count = %lu, size = %lu", count, size);
        return -1;
    }

    auto allocator = new MpAlloc(count, size);
    if (nullptr == allocator)
    {
        loge("create allocator(%lu) failed", size);
        return -1;
    }

    if (mAllocators.find(size) == mAllocators.end())
    {
        mAllocators[size] = allocator;
    }
    else
    {
        // TODO: 对已经存在的分配器进行扩容
    }

    return 0;
}

error_t MemoryPool::getAllocator(char *vp, MpAlloc **palc)
{
    for (auto it = mAllocators.begin(); it != mAllocators.end(); ++it)
    {
        auto allocator = it->second;
        if (vp > allocator->mBaseAddress && vp < allocator->mEndAddress)
        {
            *palc = allocator;
            return 0;
        }
    }

    return -1;
}
