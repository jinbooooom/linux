#include <cstring>
#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_SIZE 1024

int main()
{
    // 创建一个键值用于共享内存标识。所有进程用同一个 key 就可以了，不用关心是否是通过 ftok 生成的
    key_t key = ftok("linux", 'A');  // 如果没有文件夹 linux，将把 -1 作为 key
    printf("key = %d\n", key);

    // 连接到共享内存段
    int shmid = shmget(key, SHM_SIZE, 0666);
    // int shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid < 0) {
        std::cout << "无法连接到共享内存" << std::endl;
        return 1;
    }

    // 连接到共享内存段
    char *sharedMemory = (char *) shmat(shmid, NULL, 0);
    if (sharedMemory == (char *) -1) {
        std::cout << "无法连接到共享内存" << std::endl;
        return 1;
    }

    // 从共享内存中读取数据并输出
    std::cout << "进程2读取到的数据为: " << sharedMemory << std::endl;

    // 断开共享内存连接
    shmdt(sharedMemory);

    // 销毁共享内存标识。如果不销毁，下次运行程序，依旧得到上一次共享内存里的数据
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}

/*
g++ m1.cpp -o m1
g++ m2.cpp -o m2

先运行 ./m1
再运行 ./m2

m1 上打印：
key = 1091569648
写入数据到共享内存成功:      Hello from Process 1!

m2 上打印：
key = 1091569648
进程2读取到的数据为:      Hello from Process 1!
*/