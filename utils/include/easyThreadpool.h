#ifndef __JINBO_EASY_THREAD_POOL_H__
#define __JINBO_EASY_THREAD_POOL_H__

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <queue>
#include <stdexcept>
#include <thread>
#include <vector>

// 线程池,可以提交变参函数或拉姆达表达式的匿名函数执行,可以获取执行返回值
// 不直接支持类成员函数, 支持类静态成员函数或全局函数, operator()函数等
class EasyThreadpool
{
    using Task_t = std::function<void()>;  // 定义类型
    int mInitSize;                         // 初始化线程数量
    int mMaxSize;                          // 线程池最大容量
    std::vector<std::thread> mPool;        // 线程池
    std::queue<Task_t> mTaskQ;             // 任务队列
    std::mutex mQueueLock;                 // 任务队列同步锁
    std::mutex mGrowLock;                  // 线程池增长同步锁
    std::condition_variable mTaskCv;       // 条件阻塞
    std::atomic<bool> mRun;                // 线程池是否执行
    std::atomic<int> mUnuseThreadNum;      // 空闲线程数量
    const bool mIsAutoGrow;

public:
    inline EasyThreadpool(int initSize = 8, int maxSize = 16, bool isAutoGrow = false)
        : mInitSize(initSize), mMaxSize(maxSize), mRun(true), mUnuseThreadNum(0), mIsAutoGrow(isAutoGrow)
    {
        if (initSize <= 1) {
            mInitSize = 1;
        }

        if (mMaxSize < mInitSize) {
            mMaxSize = mInitSize;
        }

        // std::cout << mInitSize << " " << mMaxSize << " " << mUnuseThreadNum << "\n";
        addThread(mInitSize);
    }

    inline ~EasyThreadpool()
    {
        mRun = false;
        mTaskCv.notify_all();
        for (std::thread &thread : mPool) {
            // thread.detach();
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

public:
    template <class F, class... Args>
    auto commit(F &&f, Args &&...args) -> std::future<decltype(f(args...))>
    {
        if (!mRun) {
            throw std::runtime_error("commit on thread pool is stopped.");
        }

        using RetType = decltype(f(args...));
        // 把函数入口及参数, 打包(绑定)
        auto task =
            std::make_shared<std::packaged_task<RetType()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        std::future<RetType> future = task->get_future();
        {
            std::lock_guard<std::mutex> lock{mQueueLock};
            // logd("unuse thread count = %d", unuseCount());
            mTaskQ.emplace([task]() { (*task)(); });
        }

        mTaskCv.notify_one();  // 唤醒一个线程执行

        return future;
    }

    // 提交一个无参任务, 且无返回值
    template <class F>
    void commitVoid(F &&task)
    {
        if (!mRun) {
            throw std::runtime_error("commit on thread pool is stopped.");
        }

        {
            std::lock_guard<std::mutex> lock{mQueueLock};
            // logd("unuse thread count = %d", unuseCount());
            mTaskQ.emplace(std::forward<F>(task));
        }

        mTaskCv.notify_one();
    }

    // number of idle threads
    int unuseCount()
    {
        return mUnuseThreadNum;
    }

    // the number of threads
    int count()
    {
        return mPool.size();
    }

private:
    void addThread(int size)
    {
        // 增加线程数量,但不超过最大数量 mMaxSize
        for (; mPool.size() < (size_t) mMaxSize && size > 0; --size) {
            mPool.emplace_back([this] {  // work thread
                // 防止 mRun 为 false 时立即结束, 此时任务队列可能不为空，导致任务队列的任务没有执行完
                while (true) {
                    Task_t task;
                    {
                        std::unique_lock<std::mutex> lock{mQueueLock};
                        mTaskCv.wait(lock, [this] { return !mRun || !mTaskQ.empty(); });
                        if (!mRun && mTaskQ.empty()) {
                            // 当 mRun 为 false 时，依旧执行，直到任务队列为空才结束线程
                            return;
                        }
                        mUnuseThreadNum--;
                        task = move(mTaskQ.front());
                        mTaskQ.pop();
                    }
                    task();

                    {
                        std::unique_lock<std::mutex> lock{mQueueLock};
                        mUnuseThreadNum++;
                    }
                }
            });
            {
                std::unique_lock<std::mutex> lock{mQueueLock};
                mUnuseThreadNum++;
            }
        }
    }
};

#endif // __JINBO_EASY_THREAD_POOL_H__
