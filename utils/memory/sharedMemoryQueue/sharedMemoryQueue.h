#ifndef __JINBOOM_SHARED_MEMORY_QUEUE_H__
#define __JINBOOM_SHARED_MEMORY_QUEUE_H__

#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <semaphore.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

template <typename T>
class SMQueue
{
private:
    T *mBuffer;        // 共享内存缓冲区
    sem_t *mSemMutex;  // 互斥信号量
    struct SharedData {
        size_t front;  // 队头索引
        size_t rear;   // 队尾索引
    } *mPAttr;
    size_t mAllocSize;  // 用户分配的空间 + 1
    key_t mKey;

    int mShmid;
    std::string mSemName;

public:
    SMQueue(size_t size, key_t key = 6666);

    ~SMQueue();

    int Push(T value);

    int Pop();

    bool Empty();

    size_t Size();

    void Show();  // for debug
};

#include "sharedMemoryQueue.inl"

#endif