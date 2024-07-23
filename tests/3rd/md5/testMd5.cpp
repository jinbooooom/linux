#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <chrono>
#include <iostream>
#include <string>
#include "md5/md5.h"

int main(int argc, char **argv)
{

    int i;
    uint8_t result[16];           // mds number
    char resultstr[32];           // 128 bit / 4 = 32 Hexadecimal character
    const size_t size = 1 << 20;  // 1MB buffer
    uint8_t *buf      = new uint8_t[size];
    memset(buf, 1, size);

    auto t1 = std::chrono::steady_clock::now();
    md5(buf, size, result);
    auto t2   = std::chrono::steady_clock::now();
    auto time = std::chrono::duration<double, std::milli>(t2 - t1).count();
    printf("md5 %luB data spent time = %lfms\n", size, time);

    md5str(result, resultstr);
    printf("md5 hexadecimal character = %s\n", resultstr);

    delete[] buf;

    return 0;
}

/***
jinbo@fang:$ g++ md5.cpp -o md5 && ./md5
md5 1048576B data spent time = 15.864010ms
md5 hexadecimal character = d35bb2e58b602d94ccd9628f249ae7e
*/