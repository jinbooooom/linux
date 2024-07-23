#include "../log/log.hpp"
#include "easyThreadpool.h"
#include "threadpool.h"

#include <iostream>

int funIntInt(int time)
{
    if (time > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(time));
        logw(" ======= funVoidInt sleep %dms", time);
    }

    return time;
}

int main()
{
    ThreadPoolConfig thCfg;
    thCfg.thNum = 8;
    thCfg.priority = sMinPriority + 1;
    thCfg.schedule = SCHED_RR;
    thCfg.bindCpu = false;
    thCfg.enableMonitor = false;
    thCfg.monitorIntervalMs = 20;
    Threadpool thpool(thCfg);

    const int taskNum = 16;
    std::vector<std::future<int>> results(taskNum);
    for (int i = 0; i < taskNum; ++i) {
        results[i] = thpool.commit(std::bind(funIntInt, i), i);
    }
    for (int i = 0; i < taskNum; ++i) {
        logd("i = %d, result = %d", i, results[i].get());
    }

    return 0;
}


/**
 * g++ -o tp testThreadpool.cpp threadpool.cpp threadpool.h -pthread
 * 
 */
