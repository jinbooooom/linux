#include "log.h"
#include "shm.h"

#include <sys/time.h>

int main()
{
    int loop    = 6;
    size_t size = 4096;
    key_t key   = 1024;

    int pid = fork();
    if (pid > 0)  // 父进程
    {
        LxShm shm;
        shm.CreateShm(key, size);
        auto cond = shm.GetCond();
        auto lock = shm.GetLock();
        auto data = (int *)shm.GetData();
        memset(data, 0, 5);

        // data[0] = 100;
        // data[1] = 101;

        sleep(1);
        for (int i = 0; i < loop; ++i)
        {
            lock.Lock();

            data[i] = 100 + i;
            logm("signal%d, set data[%d] = %d\n", i, i, data[i]);
            cond.Notify();

            lock.Unlock();

            sleep(2);
        }

        logm("signal Process exit\n");
        shm.DestroyShm();

        exit(0);
    }
    else if (0 == pid)  // 子进程
    {
        LxShm shm;
        shm.CreateShm(key, size);
        auto cond = shm.GetCond();
        auto lock = shm.GetLock();
        auto data = (int *)shm.GetData();

        for (int i = 0; i < loop; ++i)
        {
            lock.Lock();

            cond.Wait(lock, 5, [data, i]() { return data[i] >= 100; });
            // cond.Wait(lock, 5);
            // sleep(1);
            logm("wait%d, get data[%d] = %d\n", i, i, data[i]);
            data[i] = 0;

            lock.Unlock();
        }

        loga("wait Process exit\n");
        shm.DestroyShm();

        exit(0);
    }
    else  // 出错
    {
        logm("fork failed");
        return -1;
    }

    return 0;
}

/*
first in
sizeof = 128, cona = 0x7fc5e2b08008, mutex = 0x7fc5e2b08038
sizeof = 128, cona = 0x7fc5e2b08008, mutex = 0x7fc5e2b08038
2024/07/10 18:11:23.822435 3797371584 41006 [testCond.cpp:main:31] signal0, set data[0] = 100

2024/07/10 18:11:23.822625 3797371584 41007 [testCond.cpp:main:59] wait0, get data[0] = 100

2024/07/10 18:11:25.822690 3797371584 41006 [testCond.cpp:main:31] signal1, set data[1] = 101

2024/07/10 18:11:25.822806 3797371584 41007 [testCond.cpp:main:59] wait1, get data[1] = 101

2024/07/10 18:11:27.822864 3797371584 41006 [testCond.cpp:main:31] signal2, set data[2] = 102

2024/07/10 18:11:27.822978 3797371584 41007 [testCond.cpp:main:59] wait2, get data[2] = 102

2024/07/10 18:11:29.823044 3797371584 41006 [testCond.cpp:main:31] signal3, set data[3] = 103

2024/07/10 18:11:29.823148 3797371584 41007 [testCond.cpp:main:59] wait3, get data[3] = 103

2024/07/10 18:11:31.823227 3797371584 41006 [testCond.cpp:main:31] signal4, set data[4] = 104

2024/07/10 18:11:31.823310 3797371584 41007 [testCond.cpp:main:59] wait4, get data[4] = 104

2024/07/10 18:11:33.823399 3797371584 41006 [testCond.cpp:main:31] signal5, set data[5] = 105

2024/07/10 18:11:33.823499 3797371584 41007 [testCond.cpp:main:59] wait5, get data[5] = 105

2024/07/10 18:11:33.823551 3797371584 41007 [testCond.cpp:main:65] wait Process exit

2024/07/10 18:11:35.823579 3797371584 41006 [testCond.cpp:main:39] signal Process exit
*/