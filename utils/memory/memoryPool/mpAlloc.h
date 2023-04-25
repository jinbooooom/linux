/*
 * @Author: jinboom
 * @Date: 2023-04-22 17:09:57
 * @LastEditors: jinboom
 * @LastEditTime: 2023-04-25 23:57:13
 * @FilePath: /linux/utils/memory/memoryPool/mpAlloc.h
 * @Description:
 *
 */

#ifndef __MEMORY_POOL_ALLOC_H__
#define __MEMORY_POOL_ALLOC_H__

#include <stdlib.h>
#include "../../log/log.hpp"

typedef enum
{
    MEM_FIXED_BLOCK   = 1,
    MEM_DYNAMIC_BLOCK = 4
} mem_alloc_type_t;

typedef int error_t;
typedef char mem_header_t;
// typedef char mem_tail_t;  // 存放扩容后的另一块内存区域基地址

struct FixedAlloc final
{
    explicit FixedAlloc(const size_t count, const size_t size);
    FixedAlloc()                              = delete;
    FixedAlloc(const FixedAlloc &)            = delete;
    FixedAlloc &operator=(const FixedAlloc &) = delete;
    ~FixedAlloc()                             = default;

    char *Malloc();
    error_t Free(char *vp);

    size_t mCount;
    size_t mBlockSize;
    size_t mTotalSize;
    const size_t mMemHeaderSize;
    char *mBaseAddress;
    char *mEndAddress;  // 不包含尾部数据

  private:
    bool isInvalidPointer(char *vp);
};

struct DynamicAlloc final
{
    explicit DynamicAlloc(const size_t size);
    DynamicAlloc()                                = delete;
    DynamicAlloc(const DynamicAlloc &)            = delete;
    DynamicAlloc &operator=(const DynamicAlloc &) = delete;
    ~DynamicAlloc()                               = default;

    char *Malloc(uint32_t size);
    error_t Free(char *vp);

    size_t mTotalSize;
    const size_t mMemHeaderSize;
    const size_t mMemTailSize;  // used flag(1bit) + currentSize=0(3bit) + next address(8bit)
    char *mBaseAddress;
    char *mEndAddress;
    char *mNextFitAddress;

  private:
    void setHeader(char *p, uint32_t size, uint8_t used);
    bool isInvalidPointer(char *vp);
};

#endif