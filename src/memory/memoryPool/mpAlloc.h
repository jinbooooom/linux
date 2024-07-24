/*
 * @Author: jinboom
 * @Date: 2023-04-22 17:09:57
 * @LastEditors: jinboom
 * @LastEditTime: 2023-04-22 20:41:29
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
    MEM_FIXED_BLOCK = 0,
    MEM_DYNAMIC_BLOCK
} mem_alloc_type_t;

typedef int error_t;
typedef char mem_header_t;
typedef char *mem_tail_t;  // 存放扩容后的另一块内存区域基地址

#define MEM_HEADER_SIZE (sizeof(mem_header_t))

class MpAlloc final
{
  public:
    explicit MpAlloc(const size_t count, const size_t size);
    MpAlloc()                           = delete;
    MpAlloc(const MpAlloc &)            = delete;
    MpAlloc &operator=(const MpAlloc &) = delete;
    ~MpAlloc()                          = default;

    char *Malloc();
    error_t Free(char *vp);

  private:
    bool isInvalidPointer(char *vp);

  public:
    size_t mCount;
    size_t mBlockSize;
    size_t mTotalSize;
    char *mBaseAddress;
    char *mEndAddress;  // 不包含尾部数据
};

#endif