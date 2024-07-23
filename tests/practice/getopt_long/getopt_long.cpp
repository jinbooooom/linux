
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <list>
#include <string>
#include <thread>
#include <vector>

#define CHECK(cond)                                                \
    do                                                             \
    {                                                              \
        int error = cond;                                          \
        if (error)                                                 \
        {                                                          \
            printf("%s failed!, error code = %d\n", #cond, error); \
            return error;                                          \
        }                                                          \
    } while (0);

struct config_t
{
    int size;
    int test;
    int numberOfThread;
    int duplicate;
};

void print_config(config_t config)
{
    printf(" ********** config **********\n");
    printf(" size : %d\n", config.size);
    printf(" test : %d\n", config.test);
    printf(" numberOfThread : %d\n", config.numberOfThread);
    printf(" duplicate : %d\n", config.duplicate);
    printf(" ********** config **********\n");
}

void usage(const char *argv)
{
    printf("Options:\n");
    printf(" -s, --size minimum value in sendBuf and receBuf size (default 2MB)\n");
    printf(" -t, --test test case number (default 1)\n");
    printf(" -n, --numberOfThread the number of thread. (default 1)\n");
    printf(" -d, --duplicate number of repetitions. (default 1)\n");
    printf(" -h -?, --help\n");
}

int test_1(config_t cfg)
{
    printf("run test1\n");
    return 0;
}

int test_2(config_t cfg)
{
    printf("run test2\n");
    return 1;
}

int main(int argc, char **argv)
{
    int ret = 0;

    config_t config;
    memset(&config, 0, sizeof(config_t));
    config.size           = 1;
    config.test           = 1;
    config.numberOfThread = 1;

    while (1)
    {
        static struct option long_options[] = {{.name = "size", .has_arg = 1, .flag = nullptr, .val = 's'},
                                               {.name = "test", .has_arg = 1, .flag = nullptr, .val = 't'},
                                               {.name = "numberOfThread", .has_arg = 1, .flag = nullptr, .val = 'n'},
                                               {.name = "duplicate", .has_arg = 1, .flag = nullptr, .val = 'd'},
                                               {.name = "help", .has_arg = 0, .flag = nullptr, .val = 'h'},
                                               {.name = "help", .has_arg = 0, .flag = nullptr, .val = '?'},
                                               {.name = nullptr, .has_arg = 0, .flag = nullptr, .val = '\0'}};

        /**
         * getopt 函数只能处理短选项，而 getopt_long 函数两者都可以，
         * 可以说 getopt_long 已经包含了getopt 的功能。因此，这里就只介绍 getopt_long 函数
         * 形式如 "s:t:n::h?"，分别表示程序支持的命令行短选项有 -s、-t、-n、-h，冒号含义如下：
         * 1. 只有一个字符，不带冒号——只表示选项， 如 -h, -?
         * 2. 一个字符，后接一个冒号或两个冒号，表示选项后面带一个参数，如-s 100, -t 1, -n2（中间无空格）
         */
        int c = getopt_long(argc, argv, "s:t:n::h?", long_options, nullptr);  // 不加冒号表示 has_arg = 0
        if (c == -1)
        {
            break;
        }

        switch (c)
        {
            case 's':
                config.size = strtoul(optarg, nullptr, 0);
                break;
            case 't':
                config.test = strtoul(optarg, nullptr, 0);
                break;
            case 'n':
                config.numberOfThread = strtoul(optarg, nullptr, 0);
                break;
            case 'h':
            case '?':
                usage(argv[0]);
                return -1;
            default:
                usage(argv[0]);
                return -1;
        }
    }

    print_config(config);

    auto workThread = [&](int test, config_t config) {
        switch (config.test)
        {
            case 1:
            {
                CHECK(test_1(config));
                break;
            }
            case 2:
            {
                CHECK(test_2(config));
                break;
            }

            default:
            {
                CHECK(test_1(config));
                printf("test case %d is not exist\n", config.test);
                break;
            }
        }

        return 0;
    };

    std::vector<std::thread> vecThreads;
    for (int i = 0; i < config.numberOfThread; ++i)
    {
        vecThreads.push_back(std::move(std::thread{workThread, config.test, config}));
    }

    for (int i = 0; i < config.numberOfThread; ++i)
    {
        vecThreads[i].join();
    }

    return 0;
}
