/*
 * @Author: jinboom
 * @Date: 2023-04-22 20:03:25
 * @LastEditors: jinboom
 * @LastEditTime: 2023-04-25 23:44:34
 * @FilePath: /linux/utils/memory/memoryPool/memoryPool.h
 * @Description:
 *
 */

#include <map>
#include <unordered_map>
#include "mpAlloc.h"

typedef enum
{
    MEM_4B  = 4,
    MEM_8B  = 8,
    MEM_16B = 16

} mem_block_size_t;

class MemoryPool
{
  public:
    MemoryPool();
    explicit MemoryPool(const size_t count, const size_t size, mem_alloc_type_t type);
    explicit MemoryPool(const size_t count, const size_t size);
    explicit MemoryPool(const size_t size);
    MemoryPool(const MemoryPool &)            = delete;
    MemoryPool &operator=(const MemoryPool &) = delete;
    ~MemoryPool();

    char *Malloc(size_t size);
    error_t Free(char *vp);

  private:
    error_t createAllocator(const size_t count, const size_t size, mem_alloc_type_t type = MEM_FIXED_BLOCK);
    error_t getAllocator(char *vp, FixedAlloc **allocator);

  private:
    std::map<size_t /*size*/, FixedAlloc *> mAllocators;
    DynamicAlloc *mDynamicAllocator;

  public:
    static std::map<size_t /*size*/, size_t /*count*/> sDefaultMemCfg;
};
