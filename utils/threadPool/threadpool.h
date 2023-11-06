#ifndef __JINBO_THREAD_POOL_H__
#define __JINBO_THREAD_POOL_H__

#include "../base/base.h"

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cstring>
#include <functional>
#include <future>
#include <iostream>
#include <map>
#include <queue>
#include <stdexcept>
#include <thread>
#include <unistd.h>
#include <list>
#include <vector>


#define CV_NOTIFY (1)
using Task_t = std::function<void()>;
static const int sSchedule = SCHED_OTHER;                          // SCHED_OTHER, SCHED_FIFO, SCHED_RR;
static const int sMinPriority = sched_get_priority_min(SCHED_RR);  // 1
static const int sMaxPriority = sched_get_priority_max(SCHED_RR);  // 99
static const int sCpuNum = std::thread::hardware_concurrency();
static const int sCommonThreadIndex = -1;

struct ThreadPoolConfig {
    int thNum = 8;
    int maxThreadNum = sCpuNum;
    int maxAssistantThreadNum = maxThreadNum;  // thNum * 2;
    int helpRange = 2;
    int priority = sMinPriority;
    int schedule = sSchedule;
    int maxTTL = 10;
    int cpuNum = sCpuNum;
    bool bindCpu = false;  // 当前任务不绑定 cpu 执行却更快
    bool enableMonitor = false;
    int monitorIntervalMs = 50;  // ms

    ThreadPoolConfig() = default;
    ~ThreadPoolConfig() = default;
};

static ThreadPoolConfig sDefaultThreadCfg;

class TaskQueue
{
public:
    bool empty()
    {
        return mQ.empty();
    }

    int size()
    {
        return mQ.size();
    }

    void push(Task_t &&task)
    {
        while (true) {
            if (mLock.try_lock()) {
                mQ.emplace_back(std::move(task));
                mLock.unlock();
                break;
            } else {
                std::this_thread::yield();
            }
        }
    }

    bool pop(Task_t &task)  // 不加锁
    {
        bool result = false;

        if (!mQ.empty()) {
            task = std::move(mQ.front());
            mQ.pop_front();
            result = true;
        }

        return result;
    }

    bool tryPop(Task_t &task)
    {
        bool result = false;
        if (mLock.try_lock()) {
            if (!mQ.empty()) {
                task = std::move(mQ.front());
                mQ.pop_front();
                result = true;
            }
            mLock.unlock();
        }

        return result;
    }

    bool tryHelp(Task_t &task)
    {
        bool result = false;
        if (mLock.try_lock()) {
            if (!mQ.empty()) {
                task = std::move(mQ.back());
                mQ.pop_back();
                result = true;
            }
            mLock.unlock();
        }

        return result;
    }

private:
    std::deque<Task_t> mQ;
    std::mutex mLock;
};

class WorkThread
{
public:
    typedef enum _ThreadStatus_t {
        THREAD_NOT_INIT,
        THREAD_INIT,
        THREAD_RUNNING,
        THREAD_STOP,
        THREAD_MAX
    } ThreadStatus_t;

    typedef enum _ThreadType_t { THREAD_CORE, THREAD_ASSISTANT } ThreadType_t;

public:
    explicit WorkThread(int index, ThreadType_t type, TaskQueue *poolTaskQ, std::vector<WorkThread *> *poolThreads,
                        ThreadPoolConfig *config);

    ~WorkThread();

    Error_t init();

    bool shrinkage();

protected:
    Error_t run();

    void processTask();

    bool popTask(Task_t &task);

    bool popCommonTask(Task_t &tasks);

    bool popOtherTask(Task_t &task);

protected:
    Error_t setSchedParam();

    Error_t setAffinity(int index);

    static int calcSchedule(int policy);

    static int calcPriority(int priority);

private:
    int mIndex;
    ThreadType_t mType;
    ThreadStatus_t mStatus;
    bool mIsBusy;
    TaskQueue mTaskQ;
    std::thread mWorkThread;
    int mHelpRange;  // 帮助处理的范围
    int mTTL;        // time to live 仅当 type 是辅助线程时有用
    std::mutex mQueueLock;
    std::condition_variable mTaskCv;

    // 下面的成员生命周期由线程池管理
    TaskQueue *mCommonTaskQ;
    std::vector<WorkThread *> *mPthreads;
    ThreadPoolConfig *mCfg;

    friend class Threadpool;
};

class Threadpool
{
public:
    typedef enum _ThreadPoolStatus_t {
        THREAD_POOL_NOT_INIT,
        THREAD_POOL_INIT,
        THREAD_POOL_STOP,
        THREAD_POOL_MAX
    } ThreadPoolStatus_t;

    explicit Threadpool(ThreadPoolConfig cfg = sDefaultThreadCfg) noexcept;

    explicit Threadpool(int threadNum) noexcept;

    ~Threadpool();

    Error_t init();

    template <typename F>
    auto commit(const F &f, int index = sCommonThreadIndex) -> std::future<decltype(std::declval<F>()())>;

    // template <class F, class... Args>
    // auto Threadpool::commit2(F &&f, Args &&...args) -> std::future<decltype(f(args...))>
    // {
    //     if (unlikely(THREAD_POOL_INIT != mStatus)) {
    //         std::string errorMsg = "current status(" + std::to_string(mStatus) + ") can't commit task";
    //         throw std::runtime_error(errorMsg);
    //     }

    //     return commitToThread(std::move(std::bind(std::forward<F>(f), std::forward<Args>(args)...)));
    // }

    int getThreadNum();

protected:
    int dispatch(int index);

    bool createAssistantThread(int index);

    Error_t monitor();

private:
    ThreadPoolStatus_t mStatus;
    TaskQueue mPoolTasksQ;
    std::vector<WorkThread *> mCoreThreads;
    std::list<std::unique_ptr<WorkThread>> mAssistantThreads;
    ThreadPoolConfig mCfg;
    std::thread mMonitor;
};

#include "threadpool.inl"

#endif
