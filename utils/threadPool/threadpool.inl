#ifndef __JINBO_THREAD_POOP_INL__
#define __JINBO_THREAD_POOP_INL__

#include "threadpool.h"

template <typename F>
auto Threadpool::commit(const F &f, int index) -> std::future<decltype(std::declval<F>()())>
{
    if (unlikely(THREAD_POOL_INIT != mStatus)) {
        std::string errorMsg = "current status(" + std::to_string(mStatus) + ") can't commit task";
        throw std::runtime_error(errorMsg);
    }

    using RetType = decltype(std::declval<F>()());
    auto task = std::make_shared<std::packaged_task<RetType()>>(f);
    std::future<RetType> future(task->get_future());

    int realIndex = dispatch(index);
    if (realIndex >= 0 && realIndex < mCfg.thNum) {
        // WorkThread::THREAD_RUNNING == mCoreThreads[realIndex]->mStatus) {
        // 如果返回的结果，在核心线程数量之间，则放到核心线程的queue中执行
        mCoreThreads[realIndex]->mTaskQ.push(std::move(([task]() { (*task)(); })));
#if CV_NOTIFY
        {
            std::lock_guard<std::mutex> lock(mCoreThreads[realIndex]->mQueueLock);
            mCoreThreads[realIndex]->mTaskCv.notify_one();
        }
#endif
    } else {
        // 其它不合法的索引，将任务放到公共任务池中，谁有空谁处理
        mPoolTasksQ.push(std::move([task]() { (*task)(); }));
    }

    return future;
}

#endif