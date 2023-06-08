
#include "resourceGuard.hpp"
#include <iostream>

int main()
{

    int A = 5;
    int B = 6;

    // std::function<int()>
    auto cleanA = [&]() {
        A = 0;
        return 0;
    };

    // std::function<void()>
    auto cleanB = [&]() { B = -1; };

    printf("before clean: A = %d, B = %d\n", A, B);
    {
        ResourceGuard<std::function<int()>> gcA(cleanA);
        ResourceGuard<std::function<void()>> gcB(cleanB);
    }
    printf("after clean: A = %d, B = %d\n", A, B);

    return 0;
}