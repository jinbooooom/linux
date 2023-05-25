
#ifndef __JINBO_LOG_HPP__
#define __JINBO_LOG_HPP__

#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <cstdio>
#include <list>
#include <string>
#include <vector>

#define unuse(x) ((x) = (x))
#define LOG_TIME_SIZE (128)
#define LOG_TXT_SIZE (256)
#define LOG_FULL_TXT_SIZE (512)

#define JINBO_DEBUG (1)
#define PRINT_FULL_FUNCTION_NAME (0)
#define PRINT_THREAD_PROCESS_ID_AND_FULL_TIME (1)

#if PRINT_FULL_FUNCTION_NAME
#    define FUNC_FORMAT __PRETTY_FUNCTION__
#else
#    define FUNC_FORMAT __func__
#endif

#define CHECK(cond)                                                                                                    \
    do {                                                                                                               \
        int error = cond;                                                                                              \
        if (error) {                                                                                                   \
            logd("%s failed!, error code = %d", #cond, error);                                                         \
            return error;                                                                                              \
        }                                                                                                              \
    } while (0);

static const int JINBO_LOG_LEVEL_NOTHING = 0;
static const int JINBO_LOG_LEVEL_DEBUG   = 1;
static const int JINBO_LOG_LEVEL_INFO    = 2;
static const int JINBO_LOG_LEVEL_WARNING = 3;
static const int JINBO_LOG_LEVEL_ERROR   = 4;

static int getLogLevel()
{
    char *env_log_level = getenv("JINBO_LOG_LEVEL");
    //int level            = JINBO_LOG_LEVEL_WARNING;
    int level            = JINBO_LOG_LEVEL_DEBUG;
    if (env_log_level)
    {
        try
        {
            level = std::stoi(env_log_level);
        }
        catch (...)
        {
            level = JINBO_LOG_LEVEL_WARNING;  // warning
        }

        if (level < JINBO_LOG_LEVEL_DEBUG || level > JINBO_LOG_LEVEL_ERROR)
        {
            level = JINBO_LOG_LEVEL_WARNING;
        }
    }
    return level;
}

static int logLevel = getLogLevel();

// Print only file names (filter out directories)
#define FILE_NAME(x) (strrchr((x), '/') ? strrchr((x), '/') + 1 : (x))

static thread_local char logTime[LOG_TIME_SIZE]     = {0};
static thread_local char logTxt[LOG_TXT_SIZE]       = {0};
static thread_local char fullTxt[LOG_FULL_TXT_SIZE] = {0};

static void getTimeStr(char *t)
{
    struct timeval tv;
    struct timezone tz;
    struct tm *p;

    gettimeofday(&tv, &tz);
    p = localtime(&tv.tv_sec);
#if (PRINT_THREAD_PROCESS_ID_AND_FULL_TIME)
    snprintf(t, LOG_TIME_SIZE, "%04d/%02d/%02d %02d:%02d:%02d.%06ld %u %d", p->tm_year + 1900, p->tm_mon + 1,
             p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, tv.tv_usec, (unsigned int)pthread_self(), getpid());
#else
    snprintf(t, LOG_TIME_SIZE, "%02d:%02d:%02d.%06ld", p->tm_hour, p->tm_min, p->tm_sec, tv.tv_usec);
//     snprintf(t, LOG_TIME_SIZE, "%s@%s %02d:%02d:%02d.%06ld", get_host_name().c_str(), get_local_ip().c_str(),
//              p->tm_hour, p->tm_min, p->tm_sec, tv.tv_usec);
#endif
}

#if (JINBO_DEBUG)
#    define logi(format, ...)                                                                                     \
        do                                                                                                        \
        {                                                                                                         \
            if (JINBO_LOG_LEVEL_INFO >= logLevel)                                                                 \
            {                                                                                                     \
                getTimeStr(logTime);                                                                              \
                sprintf(logTxt, format, ##__VA_ARGS__);                                                           \
                sprintf(fullTxt, "\033[1m\033[40;37m %s I [%s:%s:%d] %s \033[0m\n", logTime, FILE_NAME(__FILE__), \
                        FUNC_FORMAT, __LINE__, logTxt);                                                           \
                fprintf(stderr, "%s", fullTxt);                                                                   \
            }                                                                                                     \
        } while (0);

#    define logd(format, ...)                                                                                     \
        do                                                                                                        \
        {                                                                                                         \
            if (JINBO_LOG_LEVEL_DEBUG >= logLevel)                                                                \
            {                                                                                                     \
                getTimeStr(logTime);                                                                              \
                sprintf(logTxt, format, ##__VA_ARGS__);                                                           \
                sprintf(fullTxt, "\033[1m\033[40;32m %s D [%s:%s:%d] %s \033[0m\n", logTime, FILE_NAME(__FILE__), \
                        FUNC_FORMAT, __LINE__, logTxt);                                                           \
                fprintf(stderr, "%s", fullTxt);                                                                   \
            }                                                                                                     \
        } while (0);

#    define logw(format, ...)                                                                                     \
        do                                                                                                        \
        {                                                                                                         \
            if (JINBO_LOG_LEVEL_WARNING >= logLevel)                                                              \
            {                                                                                                     \
                getTimeStr(logTime);                                                                              \
                sprintf(logTxt, format, ##__VA_ARGS__);                                                           \
                sprintf(fullTxt, "\033[1m\033[40;33m %s W [%s:%s:%d] %s \033[0m\n", logTime, FILE_NAME(__FILE__), \
                        FUNC_FORMAT, __LINE__, logTxt);                                                           \
                fprintf(stderr, "%s", fullTxt);                                                                   \
            }                                                                                                     \
        } while (0);

#    define loge(format, ...)                                                                                     \
        do                                                                                                        \
        {                                                                                                         \
            if (JINBO_LOG_LEVEL_ERROR >= logLevel)                                                                \
            {                                                                                                     \
                getTimeStr(logTime);                                                                              \
                sprintf(logTxt, format, ##__VA_ARGS__);                                                           \
                sprintf(fullTxt, "\033[1m\033[40;31m %s E [%s:%s:%d] %s \033[0m\n", logTime, FILE_NAME(__FILE__), \
                        FUNC_FORMAT, __LINE__, logTxt);                                                           \
                fprintf(stderr, "%s", fullTxt);                                                                   \
            }                                                                                                     \
        } while (0);

#    define logmark(format, ...)                                                                                  \
        do                                                                                                        \
        {                                                                                                         \
            if (JINBO_LOG_LEVEL_DEBUG >= logLevel)                                                                \
            {                                                                                                     \
                getTimeStr(logTime);                                                                              \
                sprintf(logTxt, format, ##__VA_ARGS__);                                                           \
                sprintf(fullTxt, "\033[1m\033[40;35m %s M [%s:%s:%d] %s \033[0m\n", logTime, FILE_NAME(__FILE__), \
                        FUNC_FORMAT, __LINE__, logTxt);                                                           \
                fprintf(stderr, "%s", fullTxt);                                                                   \
            }                                                                                                     \
        } while (0);

#    define login() logi("IN")

#    define logout() logi("OUT")

#else
#    define logi(format, ...)
#    define logd(format, ...)
#    define logw(format, ...)
#    define loge(format, ...)
#    define logmark(format, ...)
#    define login(format, ...)
#    define logout(format, ...)

#endif  // end JINBO_DEBUG

#endif  // end __JINBO_LOG_H__
