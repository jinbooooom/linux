#include "backward.hpp"
#include <iostream>

#define PRINT_FUNC std::cout << "my call " << __PRETTY_FUNCTION__ << std::endl;

static void f1()
{
    PRINT_FUNC

    // 初始化backward-cpp
    backward::StackTrace st;
    st.load_here(32);  // 捕获最多32帧

    backward::Printer pp;
    pp.print(st);

    int *p = new int;
    *p = 3;
    std::cout << "*p = " << *p << std::endl;
    delete p;
    p = nullptr;
    int n = *p;
}

static void f2()
{
    PRINT_FUNC;

    f1();
}

int main()
{
    PRINT_FUNC;

    f2();

    return 0;
}