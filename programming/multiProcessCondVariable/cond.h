#ifndef __JINBO_COND__
#define __JINBO_COND__

#include <pthread.h>
#include <stdio.h>
#include <sys/time.h>
#include <cerrno>

class LxCondVariable;

class LxLock
{
  public:
    LxLock(pthread_mutex_t *mutex);

    ~LxLock();

    int Lock();

    int Unlock();

    friend class LxCondVariable;

  private:
    pthread_mutex_t *mMutex;
};

class LxCondVariable
{
  public:
    LxCondVariable(pthread_cond_t *cond);

    ~LxCondVariable();

    int Notify();

    int Wait(LxLock &lock, int second = 0);

    template <typename _Cond>
    int Wait(LxLock &lock, int second, _Cond cond)
    {
        struct timespec timeout;
        struct timeval now;
        gettimeofday(&now, nullptr);
        timeout.tv_sec  = now.tv_sec + second;
        timeout.tv_nsec = now.tv_usec * 1000;

        int ret = 0;
        while (!cond())
        {
            ret = pthread_cond_timedwait(mCV, lock.mMutex, &timeout);
            if (ret == 0)
            {
                // OK
            }
            else if (ret == ETIMEDOUT)
            {
                printf("Conditional variable waiting timeout, timeout time is %d seconds.\n", second);
                break;
            }
            else
            {
                printf("Conditional variable error occurred, error = %d\n", ret);
                break;
            }
        }

        return ret;
    }

  private:
    pthread_cond_t *mCV;
};

#endif
