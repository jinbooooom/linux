#ifndef __JINBO_SHM__
#define __JINBO_SHM__

#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdalign.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string>

#include "cond.h"

class LxShm final
{
  public:
    struct SyncData
    {
        int flag;
        pthread_cond_t cond;
        pthread_mutex_t mutex;
        char test[32];
    } __attribute__((aligned(8)));

  public:
    LxShm();
    LxShm(const LxShm &)            = delete;
    LxShm &operator=(const LxShm &) = delete;
    virtual ~LxShm();

  public:
    int CreateShm(const key_t key, size_t size);

    int DestroyShm();

    char *GetData();

    LxLock GetLock();

    LxCondVariable GetCond();

  private:
    key_t key;
    int mShmid;
    char *mAddr;
    char *mData;
    size_t mSize;
    pthread_mutex_t *mMutex;
    pthread_cond_t *mCond;
};

#endif
