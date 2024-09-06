#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <thread>
#include <unistd.h>

int mShmid;

void deleteSharedMemory()
{
    printf("Deleting shared memory...\n");
    shmctl(mShmid, IPC_RMID, NULL);
}

int main()
{
    // 创建共享内存
    mShmid = shmget(IPC_PRIVATE, 1024, IPC_CREAT | 0666);
    if (mShmid == -1) {
        perror("shmget");
        return 1;
    }

    // 创建多个线程同时删除共享内存
    std::thread t1(deleteSharedMemory);
    std::thread t2(deleteSharedMemory);
    std::thread t3(deleteSharedMemory);

    t1.join();
    t2.join();
    t3.join();

    printf("All threads completed.\n");

    return 0;
}