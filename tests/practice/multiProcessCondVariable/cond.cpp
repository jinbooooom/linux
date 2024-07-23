#include "cond.h"

LxLock::LxLock(pthread_mutex_t *mutex)
{
    mMutex = mutex;
}

LxLock::~LxLock()
{
    mMutex = nullptr;
}

int LxLock::Lock()
{
    if (mMutex == nullptr)
    {
        printf("lock failed, invalid mutex\n");
        return -1;
    }

    return pthread_mutex_lock(mMutex);
    ;
}

int LxLock::Unlock()
{
    if (mMutex == nullptr)
    {
        printf("unlock failed, invalid mutex\n");
        return -1;
    }

    return pthread_mutex_unlock(mMutex);
}

LxCondVariable::LxCondVariable(pthread_cond_t *cond)
{
    mCV = cond;
}

LxCondVariable::~LxCondVariable()
{
    mCV = nullptr;
};

int LxCondVariable::Notify()
{
    return pthread_cond_signal(mCV);
}

int LxCondVariable::Wait(LxLock &lock, int second)
{
    struct timespec timeout;
    struct timeval now;
    gettimeofday(&now, nullptr);
    timeout.tv_sec  = now.tv_sec + second;
    timeout.tv_nsec = now.tv_usec * 1000;

    int ret = pthread_cond_timedwait(mCV, lock.mMutex, &timeout);
    if (ret == 0)
    {
        // OK
    }
    else if (ret == ETIMEDOUT)
    {
        printf("Conditional variable waiting timeout, timeout time is %d seconds.\n", second);
    }
    else
    {
        printf("Conditional variable error occurred, error = %d\n", ret);
    }

    return ret;
}