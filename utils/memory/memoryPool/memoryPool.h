/*
 * @Author: jinboom
 * @Date: 2023-04-22 20:03:25
 * @LastEditors: jinboom
 * @LastEditTime: 2023-04-22 22:26:13
 * @FilePath: /linux/utils/memory/memoryPool/memoryPool.h
 * @Description:
 *
 */

#include <unordered_map>
#include <map>
#include "mpAlloc.h"

typedef enum {
    MEM_4B    = 4,
    MEM_8B    = 8,
    MEM_16B    = 16

} mem_block_size_t;

class MemoryPool
{
  public:
    MemoryPool();
    explicit MemoryPool(const size_t count, const size_t size);
    MemoryPool(const MemoryPool &)            = delete;
    MemoryPool &operator=(const MemoryPool &) = delete;
    ~MemoryPool();

    char *Malloc(size_t size);
    error_t Free(char *vp);

  private:
    error_t createAllocator(const size_t count, const size_t size);
    error_t getAllocator(char *vp, MpAlloc **allocator);



  private:
     std::unordered_map<size_t /*size*/, MpAlloc *> mAllocators;

    public:
  static std::map<size_t /*size*/, size_t /*count*/> sDefaultMemCfg;
};
