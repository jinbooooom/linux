

#include "threadpool.h"
#include "../log/log.hpp"
#include "../base/base.h"

#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>


WorkThread::WorkThread(int index, ThreadType_t type, TaskQueue *poolTaskQ, std::vector<WorkThread *> *poolThreads,
                       ThreadPoolConfig *config)
    : mIndex(index), mType(type), mStatus(THREAD_NOT_INIT), mIsBusy(false), mCommonTaskQ(poolTaskQ),
      mPthreads(poolThreads), mCfg(config)
{

    // init(); // init 由外部线程池在创建工作线程后，再统一执行
}

WorkThread::~WorkThread()
{
    mStatus = THREAD_STOP;
    mTTL = -999;
    if (mWorkThread.joinable()) {
        mWorkThread.join();
    }

    logi("exit thread index%d, type(%d)", mIndex, mType);
}

Error_t WorkThread::init()
{
    if (unlikely((THREAD_NOT_INIT) != mStatus)) {
        loge("invalid thread status %d, maybe has init", mStatus);
        return ERRORPARAM;
    }

    mStatus = THREAD_INIT;
    mHelpRange = mCfg->helpRange;
    mTTL = mCfg->maxTTL;
    mWorkThread = std::move(std::thread(&WorkThread::run, this));
    // setSchedParam();
    // setAffinity(mIndex);

    // pthread_t thread = mWorkThread.native_handle();
    // int schedule = -1;
    // struct sched_param param;
    // if (pthread_getschedparam(thread, &schedule, &param) == 0) {
    //     logw("thread%d, current schedule = %d, cfg schedule = %d,  current priority = %d, cfg priority = %d",
    //           mIndex, schedule, mCfg->schedule, param.sched_priority, calcPriority(mCfg->priority));
    // } else {
    //     logi("failed to get thread priority.");
    // }

    return 0;
}

bool WorkThread::shrinkage()
{
    if (likely(mIsBusy)) {
        mTTL++;
        mTTL = std::min(mTTL, mCfg->maxTTL);
    } else {
        mTTL--;
    }

    return mTTL <= 0;
}

Error_t WorkThread::run()
{
    if (likely(THREAD_INIT == mStatus)) {
        mStatus = THREAD_RUNNING;
    } else {
        loge("current thread not init");
        return ERRORPARAM;
    }

    if (unlikely(nullptr == mPthreads)) {
        loge("invalid thread pool");
        return ERRORPARAM;
    }

    if (unlikely(std::any_of(mPthreads->begin(), mPthreads->end(), [](WorkThread *thd) { return nullptr == thd; }))) {
        loge("core thread is null");
        return ERRORPARAM;
    }

    uint32_t tid = static_cast<uint32_t>(syscall(SYS_gettid));
    logd("thread index = %d, tid = %u", mIndex, tid);

    while (THREAD_RUNNING == mStatus || !mTaskQ.empty()) {
#if CV_NOTIFY
        {
            std::unique_lock<std::mutex> lock{mQueueLock};
            // 加了超时时间是因为，如果过了超时时间还没有接到分给本线程的任务，就可以协助其他线程
            mTaskCv.wait_for(lock, std::chrono::microseconds(15),
                             [this] { return !(THREAD_RUNNING == mStatus) || !mTaskQ.empty(); });
            if (unlikely(!(THREAD_RUNNING == mStatus) && mTaskQ.empty())) {
                // 当不处于运行状态时，依旧执行，直到任务队列为空才结束线程
                return 0;
            }
        }
#endif

        processTask();
    }

    return 0;
}

void WorkThread::processTask()
{
    // static bool getTask = true;
    // static int yieldCnt = 0;
    // static auto t1 = std::chrono::steady_clock::now();
    // static auto t2 = std::chrono::steady_clock::now();
    // static auto time = std::chrono::duration<double, std::micro>(t2 - t1).count();

    Task_t task;
    if (popTask(task) || popCommonTask(task) || popOtherTask(task)) {
        // getTask = true;
        // if (1 == mIndex) {
        //     auto t2 = std::chrono::steady_clock::now();
        //     time = std::chrono::duration<double, std::micro>(t2 - t1).count();
        //     logd("idx%d yield cnt = %d, yield time %lfus", mIndex, yieldCnt, time);
        //     yieldCnt = 0;
        // }
        mIsBusy = true;
        if (likely(task)) {
            task();
        } else {
            loge("thread%d task is empty()", mIndex);
        }
        mIsBusy = false;
    } else {
        // if (getTask && (1 == mIndex)) {
        //     t1 = std::chrono::steady_clock::now();
        //     getTask = false;
        // }
        // yieldCnt++;
        // usleep(10);

        std::this_thread::yield();
    }
}

bool WorkThread::popTask(Task_t &task)
{
    return mTaskQ.tryPop(task);
}

bool WorkThread::popCommonTask(Task_t &tasks)
{
    bool result = mCommonTaskQ->tryPop(tasks);
    return result;
}

bool WorkThread::popOtherTask(Task_t &task)
{
    if (unlikely(nullptr == mCfg || (int) mPthreads->size() < mCfg->thNum)) {
        logw("param invalid: cfg = %p, thread size(%lu) < mCfg->thNum(%d)", mCfg, mPthreads->size(), mCfg->thNum);
        return false;
    }

    if (unlikely(THREAD_RUNNING != mStatus)) {
        return false;
    }

    for (int i = 0; i < mHelpRange; i++) {
        if (mTaskQ.empty()) {
            int curIndex = (mIndex + i + 1) % mCfg->thNum;
            if (unlikely(nullptr != (*mPthreads)[curIndex]) && ((*mPthreads)[curIndex])->mTaskQ.tryHelp(task)) {
                // loge("thread task = %d, thread%d got task from thread%d", mTaskQ.size(), mIndex, curIndex);
                return true;
            }
        }
    }

    return false;
}

Error_t WorkThread::setSchedParam()
{
    int priority = THREAD_CORE == mType ? calcPriority(mCfg->priority) : sMinPriority;
    int schedule = calcSchedule(mCfg->schedule);
    auto threadHandle = mWorkThread.native_handle();
    sched_param param = {priority};
    int ret = pthread_setschedparam(threadHandle, schedule, &param);
    if (0 != ret) {
        logw("set thread schedule failed, %s: threadHandle = %lu, schedule = %d, priority = %d", strerror(ret),
              threadHandle, schedule, priority);
        return ret;
    }

    return 0;
}

Error_t WorkThread::setAffinity(int index)
{
    if (!mCfg->bindCpu) {
        return 0;
    }

    if (mCfg->cpuNum == 0 || index < 0) {
        loge("invalid param: cpu num = %d, index = %d", mCfg->cpuNum, index);
        return ERRORPARAM;
    }

    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(index % mCfg->cpuNum, &mask);
    auto threadHandle = mWorkThread.native_handle();
    int ret = pthread_setaffinity_np(threadHandle, sizeof(cpu_set_t), &mask);
    if (0 != ret) {
        logw("set thread affinity failed, %s", strerror(ret));
        return ret;
    }

    return 0;
}

int WorkThread::calcSchedule(int policy)
{
    return (policy >= SCHED_OTHER && policy <= SCHED_RR) ? policy : SCHED_OTHER;
}

int WorkThread::calcPriority(int priority)
{
    return (priority >= sMinPriority && priority <= sMaxPriority) ? priority : sMinPriority;
}

Threadpool::Threadpool(ThreadPoolConfig cfg) noexcept : mStatus(THREAD_POOL_NOT_INIT)
{
    mCfg = cfg;

    init();
}

Threadpool::Threadpool(int threadNum) noexcept : mStatus(THREAD_POOL_NOT_INIT)
{
    ThreadPoolConfig cfg = sDefaultThreadCfg;
    cfg.thNum = threadNum;
    mCfg = cfg;

    init();
}

Threadpool::~Threadpool()
{
    mStatus = THREAD_POOL_STOP;
    logi("prepare clean resource");

    if (mMonitor.joinable()) {
        mMonitor.join();
    }
    logi("exit monitor thread");

    for (auto &x : mCoreThreads) {
        if (likely(x)) {
            delete x;
            x = nullptr;
        }
    }
    logi("exit Threadpool");
}

Error_t Threadpool::init()
{
    if (THREAD_POOL_NOT_INIT != mStatus) {
        loge("thread pool has init, can't init again.\n");
        return ERRORPARAM;
    }

    mCoreThreads.reserve(mCfg.thNum);
    for (int i = 0; i < mCfg.thNum; i++) {
        auto pth = new WorkThread(i, WorkThread::THREAD_CORE, &mPoolTasksQ, &mCoreThreads, &mCfg);
        mCoreThreads.emplace_back(pth);
    }

    for (int i = 0; i < mCfg.thNum; i++) {
        CHECK_ERROR(mCoreThreads[i]->init());
    }

    mStatus = THREAD_POOL_INIT;
    mMonitor = std::move(std::thread(&Threadpool::monitor, this));
    return 0;
}

int Threadpool::getThreadNum()
{
    return mCoreThreads.size() + mAssistantThreads.size();
}

int Threadpool::dispatch(int index)
{
    int realIdx = index;
    // if (likely(index < mCfg.thNum || index >= 0)) {
    if (likely(index >= 0)) {
        realIdx = index % mCfg.thNum;
    } else if (sCommonThreadIndex == index) {
        realIdx = sCommonThreadIndex;
    } else {
        realIdx = sCommonThreadIndex;
    }

    return realIdx;
}

bool Threadpool::createAssistantThread(int index)
{
    auto pth = std::make_unique<WorkThread>(index, WorkThread::THREAD_ASSISTANT, &mPoolTasksQ, &mCoreThreads, &mCfg);
    pth->init();
    mAssistantThreads.emplace_back(std::move(pth));
    logw("create assistant thread%d", index);

    return true;
}

Error_t Threadpool::monitor()
{
    int theBusiestThreadIndex = mCfg.thNum - 1;  // 0;
    int theBusiestThreadTaskCnt = 0;
    const int onceSleepTimeMs = 20;
    const int sleepCnt = mCfg.monitorIntervalMs / onceSleepTimeMs + !!(mCfg.monitorIntervalMs % onceSleepTimeMs);
    uint64_t monitorCnt = -1;

    while (THREAD_POOL_INIT >= mStatus && mCfg.enableMonitor) {
        monitorCnt++;
        while (mCfg.enableMonitor && THREAD_POOL_INIT != mStatus) {
            // 如果没有 init，则等待 init 完成
            logi("wait threadpool init");
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }

        int sleepCntTmp = sleepCnt;
        while (mCfg.enableMonitor && THREAD_POOL_INIT == mStatus && sleepCntTmp--) {
            // 将一个长的时间间隔拆分成多个小的时间间隔去休眠，是为了保证当线程池状态改变时，监控线程可以快速退出
            std::this_thread::sleep_for(std::chrono::milliseconds(onceSleepTimeMs));
        }

        int assistantThreadNum = mAssistantThreads.size();
        int leftSize = std::min(mCfg.maxThreadNum - mCfg.thNum - assistantThreadNum,
                                mCfg.maxAssistantThreadNum - assistantThreadNum);
        leftSize = std::max(0, leftSize);
        if (unlikely(0 == leftSize)) {
            if (0 == monitorCnt % 50) {
                logw("The number of assistant threads reaches the maximum: maxThreadNum = %d, "
                      "maxAssistantThreadNum = %d, current number of assistant threads = %d",
                      mCfg.maxThreadNum, mCfg.maxAssistantThreadNum, assistantThreadNum);
            }
            continue;
        }

        // 如果核心线程都在执行，则表示忙碌
        theBusiestThreadIndex = theBusiestThreadTaskCnt = 0;
        bool busy = true;
        for (auto &pth : mCoreThreads) {
            if (theBusiestThreadTaskCnt < pth->mTaskQ.size()) {
                // logd("theBusiestThreadTaskCnt(%d) <= pth->mTaskQ.size()(%d), "
                //        "theBusiestThreadIndex(%d),pth->mIndex(%d)",
                //        theBusiestThreadTaskCnt, pth->mTaskQ.size(), theBusiestThreadIndex, pth->mIndex);

                theBusiestThreadTaskCnt = pth->mTaskQ.size();
                theBusiestThreadIndex = pth->mIndex;
            }

            busy &= WorkThread::THREAD_RUNNING == pth->mStatus && pth->mIsBusy && !pth->mTaskQ.empty();
            // TODO: 这是判断全局 busy 的，可能不是一个最优的策略，如果当前线程是 busy 的，且 task
            // 积累的特别多，其他远处的线程以及辅助线程帮不上忙，应该立即创建辅助当前线程的辅助线程。
        };

        // 当核心线程全部 busy，或者公共任务池的任务足够多时，就创建辅助线程分担压力
        if (busy || mPoolTasksQ.size() >= getThreadNum()) {
            createAssistantThread(theBusiestThreadIndex);
        }

        for (auto iter = mAssistantThreads.begin(); iter != mAssistantThreads.end();) {
            (*iter)->shrinkage() ? mAssistantThreads.erase(iter++) : iter++;
        }
    }

    for (auto iter = mAssistantThreads.begin(); iter != mAssistantThreads.end();) {
        mAssistantThreads.erase(iter++);
    }

    logi("exit moniter thread");

    return 0;
}
