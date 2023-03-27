
#include "log.hpp"

#include <iostream>
#include <thread>
#include <vector>

void threadWork(int flag)
{
    for (int i = 0; i < 100; ++i)
    {
        if (flag % 2)
        {
            logd("thread%d i = %d", flag, i);
        }
        else
        {
            loge("thread%d i = %d", flag, i);
        }
    }
}

int main()
{
    const int threadNum = 5;
    std::vector<std::thread> vecThreads;
    for (int i = 0; i < threadNum; ++i)
    {
        vecThreads.push_back(std::move(std::thread{threadWork, i}));
    }

    for (int i = 0; i < threadNum; ++i)
    {
        vecThreads[i].join();
    }

    int level = getLogLevel();
    std::cout << "log level = " << level << std::endl;

    return 0;
}
