#include "../log/log.hpp"
#include "threadpool.h"

#include <iostream>

void funVoid()
{
    logi("i am funVoid");
}

void funVoidInt(int time)
{
    if (time > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(time));
        logw(" ======= funVoidInt sleep %dms", time);
    }
}

int funIntInt(int time)
{
    if (time > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(time));
        logw(" ======= funVoidInt sleep %dms", time);
    }

    return time;
}

struct funOperator {
    int operator()(int n)
    {
        logi("funOperator param: n = %d", n);
        return 42;
    }
};

class A
{
public:
    // 函数必须是 static 的才能使用线程池
    static int fun1(int n = 0)
    {
        logd("A::fun1 param: n = %d", n);
        return n;
    }

    static std::string fun2(int n, std::string str, char c)
    {
        logd("A::fun2 param: n = %d, str = %s, c = %c", n, str.c_str(), c);
        return str;
    }
};

auto main() -> int
{
    Threadpool pool{4};
    A a;

    logmark("========= test basic =========");
    std::future<void> fvi = pool.commit(funVoidInt, 3000);
    std::future<int> fo = pool.commit(funOperator{}, 0);
    std::future<int> fAf1 = pool.commit(a.fun1, 9999);
    std::future<std::string> fAf2 = pool.commit(A::fun2, 9998, "mult args", 'C');
    std::future<std::string> fh = pool.commit([]() -> std::string {
        logd("lambda test");
        return "lambda test";
    });

    std::future<void> fv = pool.commit(funVoid);
    pool.commitVoid(funVoid);
    fvi.get();

    logmark("========= other test =========");
    // std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    const int testCnt = 8;
    std::vector<std::future<int>> results;
    for (int i = 0; i < testCnt; ++i) {
        results.emplace_back(pool.commit(funIntInt, i * 500));
    }
    for (int i = 0; i < testCnt; ++i) {
        logd("i = %d, result = %d", i, results[i].get());
    }

    return 0;
}
