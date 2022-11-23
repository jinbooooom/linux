
#ifndef __JINBO_LOG_H__
#define __JINBO_LOG_H__

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>

#define unuse(x) ((x) = (x))
#define LOG_TIME_SIZE (128)

#define DEBUG (1)
#define PRINT_FULL_FUNCTION_NAME (0)
#define PRINT_THREAD_PROCESS_ID_AND_FULL_TIME (1)

#if PRINT_FULL_FUNCTION_NAME
#define FUNC_FORMAT __PRETTY_FUNCTION__
#else
#define FUNC_FORMAT __func__
#endif

// Print only file names (filter out directories)
#define FILE_NAME(x) (strrchr((x), '/') ? strrchr((x), '/') + 1 : (x))

static char logTime[LOG_TIME_SIZE] = {0};

static void getTimeStr(char *t)
{
      struct timeval tv;
      struct timezone tz;
      struct tm *p;

      gettimeofday(&tv, &tz);
      p = localtime(&tv.tv_sec);
#if (PRINT_THREAD_PROCESS_ID_AND_FULL_TIME)
      snprintf(t, LOG_TIME_SIZE, "%04d/%02d/%02d %02d:%02d:%02d.%06ld %u %d",
               p->tm_year + 1900, p->tm_mon + 1, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, tv.tv_usec,
               (unsigned int)pthread_self(), getpid());
#else
      snprintf(t, LOG_TIME_SIZE, "%02d:%02d:%02d.%06ld",
               p->tm_hour, p->tm_min, p->tm_sec, tv.tv_usec);
#endif
}

#if (DEBUG)
#define logi(format, ...)                                                                                          \
      getTimeStr(logTime);                                                                                         \
      fprintf(stdout, "\033[1m\033[40;37m %s I [%s:%s:%d] ", logTime, FILE_NAME(__FILE__), FUNC_FORMAT, __LINE__); \
      fprintf(stdout, format, ##__VA_ARGS__);                                                                      \
      fprintf(stdout, "\033[0m\n");

#define logd(format, ...)                                                                                          \
      getTimeStr(logTime);                                                                                         \
      fprintf(stdout, "\033[1m\033[40;32m %s D [%s:%s:%d] ", logTime, FILE_NAME(__FILE__), FUNC_FORMAT, __LINE__); \
      fprintf(stdout, format, ##__VA_ARGS__);                                                                      \
      fprintf(stdout, "\033[0m\n");

#define logw(format, ...)                                                                                          \
      getTimeStr(logTime);                                                                                         \
      fprintf(stdout, "\033[1m\033[40;33m %s W [%s:%s:%d] ", logTime, FILE_NAME(__FILE__), FUNC_FORMAT, __LINE__); \
      fprintf(stdout, format, ##__VA_ARGS__);                                                                      \
      fprintf(stdout, "\033[0m\n");

#define loge(format, arg...)                                                                                       \
      getTimeStr(logTime);                                                                                         \
      fprintf(stdout, "\033[1m\033[40;31m %s E [%s:%s:%d] ", logTime, FILE_NAME(__FILE__), FUNC_FORMAT, __LINE__); \
      fprintf(stdout, format, ##arg);                                                                              \
      fprintf(stdout, "\033[0m\n");

#define logmark(format, arg...)                                                                                    \
      getTimeStr(logTime);                                                                                         \
      fprintf(stdout, "\033[1m\033[40;35m %s M [%s:%s:%d] ", logTime, FILE_NAME(__FILE__), FUNC_FORMAT, __LINE__); \
      fprintf(stdout, format, ##arg);                                                                              \
      fprintf(stdout, "\033[0m \n");

#define login() logi("IN")

#define logout() logi("OUT")

#else
#define logi(format, ...)
#define logd(format, ...)
#define logw(format, ...)
#define loge(format, ...)
#define logmark(format, ...)
#define login(format, ...)
#define logout(format, ...)

#endif // end DEBUG

#endif // end __JINBO_LOG_H__
