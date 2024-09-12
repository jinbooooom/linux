#include "memoryPoolManager.h"

void threadFunction(MemoryPoolManager &pool)
{
    void *ptr1 = pool.Alloc(4);
    if (ptr1) {
        std::cout << "Thread " << std::this_thread::get_id() << " allocated 4 bytes at " << ptr1 << std::endl;
    } else {
        std::cout << "Thread " << std::this_thread::get_id() << " failed to Alloc 4 bytes" << std::endl;
    }

    void *ptr2 = pool.Alloc(4 * 1024 * 1024);
    if (ptr2) {
        std::cout << "Thread " << std::this_thread::get_id() << " allocated 4MB at " << ptr2 << std::endl;
    } else {
        std::cout << "Thread " << std::this_thread::get_id() << " failed to Alloc 4MB" << std::endl;
    }

    pool.Free(ptr1);
    pool.Free(ptr2);
}

int main()
{
    MemoryPoolManager pool(
        1024 * 1024 * 1024,
        {4, 16, 64, 256, 1024, 4 * 1024 * 1024});  // 内存池最大大小为 1GB，块大小分别为 4B, 16B, 64B, 256B, 1KB, 4MB

    std::thread t1(threadFunction, std::ref(pool));
    std::thread t2(threadFunction, std::ref(pool));

    t1.join();
    t2.join();

    return 0;
}