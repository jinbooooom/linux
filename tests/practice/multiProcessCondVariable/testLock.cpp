#include "log.h"
#include "shm.h"

#include <sys/time.h>

int main()
{
    int loop    = 100000;
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

        for (int i = 0; i < loop; ++i)
        {
            lock.Lock();
            data[0] += 1;
            lock.Unlock();
        }

        logm("signal Process exit, data[0] = %d\n", data[0]);
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
            data[0] += 1;
            lock.Unlock();
        }

        loga("wait Process exit, data[0] = %d\n", data[0]);
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
sizeof = 128, cona = 0x7eff86948008, mutex = 0x7eff86948038
sizeof = 128, cona = 0x7eff86948008, mutex = 0x7eff86948038
2024/07/10 18:03:36.865238 2252032704 38501 [testLock.cpp:main:28] signal Process exit, data[0] = 156266
 
2024/07/10 18:03:36.866217 2252032704 38502 [testLock.cpp:main:48] wait Process exit, data[0] = 200000
*/