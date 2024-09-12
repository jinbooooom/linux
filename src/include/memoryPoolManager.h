#ifndef __MEMORY_POOL_H__
#define __MEMORY_POOL_H__

#include <cstdlib>
#include <iostream>
#include <list>
#include <mutex>
#include <set>
#include <thread>
#include <vector>

class MemoryPool
{
public:
    struct MemoryBlock {
        void *ptr;
        size_t size;
    };

    MemoryPool(size_t maxSize, size_t blockSize) : maxSize(maxSize), blockSize(blockSize), currentSize(0) {}

    ~MemoryPool()
    {
        std::lock_guard<std::mutex> lock(mutex);
        for (auto &block : allocatedBlocks) {
            free(block.ptr);
            // printf("allocatedBlocks %p\n", block.ptr);
        }
        for (auto &block : freeBlocks) {
            free(block.ptr);
            // printf("freeBlocks %p\n", block.ptr);
        }
    }

    void *Alloc(size_t size)
    {
        std::lock_guard<std::mutex> lock(mutex);

        // 查找未使用的内存块
        for (auto it = freeBlocks.begin(); it != freeBlocks.end(); ++it) {
            if (it->size >= size) {
                void *ptr = it->ptr;
                allocatedBlocks.push_back(*it);
                freeBlocks.erase(it);
                return ptr;
            }
        }

        // 如果没有合适的内存块且内存池未满，分配新的内存块
        if (currentSize + blockSize <= maxSize) {
            void *ptr = malloc(blockSize);
            if (ptr) {
                allocatedBlocks.push_back({ptr, blockSize});
                currentSize += blockSize;
                return ptr;
            }
        }

        // 内存池已满或分配失败
        return nullptr;
    }

    void Free(void *ptr)
    {
        std::lock_guard<std::mutex> lock(mutex);
        for (auto it = allocatedBlocks.begin(); it != allocatedBlocks.end(); ++it) {
            if (it->ptr == ptr) {
                freeBlocks.push_back(*it);
                allocatedBlocks.erase(it);
                return;
            }
        }
    }

    size_t GetBlockSize() const
    {
        return blockSize;
    }

    bool IsPtrInPool(void *ptr) const
    {
        for (const auto &block : allocatedBlocks) {
            if (block.ptr == ptr) {
                return true;
            }
        }
        for (const auto &block : freeBlocks) {
            if (block.ptr == ptr) {
                return true;
            }
        }
        return false;
    }

private:
    size_t maxSize;
    size_t blockSize;
    size_t currentSize;
    std::list<MemoryBlock> allocatedBlocks;
    std::list<MemoryBlock> freeBlocks;
    std::mutex mutex;
};

class MemoryPoolManager
{
public:
    MemoryPoolManager(size_t maxSize, const std::set<size_t> &blockSizes)
    {
        for (size_t blockSize : blockSizes) {
            pools.emplace_back(new MemoryPool(maxSize, blockSize));
        }
    }

    ~MemoryPoolManager()
    {
        for (auto &pool : pools) {
            delete pool;
            pool = nullptr;
        }
    }

    void *Alloc(size_t size)
    {
        for (auto &pool : pools) {
            if (nullptr == pool) {
                continue;
            }
            if (size <= pool->GetBlockSize()) {
                return pool->Alloc(size);
            }
        }
        return nullptr;
    }

    void Free(void *ptr)
    {
        for (auto &pool : pools) {
            if (nullptr == pool) {
                continue;
            }
            if (pool->IsPtrInPool(ptr)) {
                pool->Free(ptr);
                return;
            }
        }
    }

private:
    std::vector<MemoryPool *> pools;
};

#endif