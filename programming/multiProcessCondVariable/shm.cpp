#include "shm.h"
#include "cond.h"

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>

void *alignToByte(void *address, size_t alignment)
{
    uintptr_t addr = reinterpret_cast<uintptr_t>(address);

    if (addr % alignment != 0)
    {
        addr += alignment - (addr % alignment);
    }

    return reinterpret_cast<void *>(addr);
}

LxShm::LxShm() {}

LxShm::~LxShm() {}

int LxShm::CreateShm(const key_t key, size_t size)
{
    if (key <= 0 || size <= 0)
    {
        printf("invalid param: key = %d, size = %lu", key, size);
        return -1;
    }

    mSize  = size;
    mShmid = shmget(key, mSize, IPC_CREAT | 0666);
    if (mShmid < 0)
    {
        printf("Unable to create shared memory, key = %d, size = %lu, %s\n", key, mSize, strerror(errno));
        return -1;
    }

    mAddr = (char *)shmat(mShmid, nullptr, 0);
    if (((mAddr == (char *)-1)))
    {
        printf("Unable to connect to shared memory.\n");
        return -1;
    }

    auto data = (SyncData *)mAddr;
    mCond     = &data->cond;
    mMutex    = &data->mutex;

    int *p = &data->flag;
    if (*p != 888)
    {
        memset(data->test, 0, sizeof(data->test));
        printf("first in\n");
        *p = 888;

        // 初始化一个进程间通信的锁和等待变量
        pthread_condattr_t cond_attr;
        pthread_condattr_init(&cond_attr);
        pthread_condattr_setpshared(&cond_attr, PTHREAD_PROCESS_SHARED);
        pthread_cond_init(mCond, &cond_attr);

        // 初始化一个可以在进程间使用的锁
        pthread_mutexattr_t mutex_attr;
        pthread_mutexattr_init(&mutex_attr);
        pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(mMutex, &mutex_attr);
    }
    printf("sizeof = %lu, cona = %p, mutex = %p\n", sizeof(SyncData), mCond, mMutex);

    return 0;
}

int LxShm::DestroyShm()
{
    shmdt(mAddr);

    shmctl(mShmid, IPC_RMID, NULL);

    mAddr = nullptr;

    return 0;
}

char *LxShm::GetData()
{
    auto data = (SyncData *)mAddr;
    return data->test;
}

LxLock LxShm::GetLock()
{
    return std::move(LxLock(mMutex));
}

LxCondVariable LxShm::GetCond()
{
    return std::move(LxCondVariable(mCond));
}