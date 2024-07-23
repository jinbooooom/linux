#define SPDLOG_ACTIVE_LEVEL \
    SPDLOG_LEVEL_TRACE  // 必须定义这个宏,才能输出文件名和行号，避免 spdlog 先定义该值，所以定义放在
                        // include spd

#include "spdlog/cfg/env.h"   // support for loading levels from the environment variable
#include "spdlog/fmt/ostr.h"  // support for user defined types
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

#define logt(format, ...) SPDLOG_LOGGER_TRACE(&logger, format, ##__VA_ARGS__)
#define logd(format, ...) SPDLOG_LOGGER_DEBUG(&logger, format, ##__VA_ARGS__)
#define logi(format, ...) SPDLOG_LOGGER_INFO(&logger, format, ##__VA_ARGS__)
#define logw(format, ...) SPDLOG_LOGGER_WARN(&logger, format, ##__VA_ARGS__)
#define loge(format, ...) SPDLOG_LOGGER_ERROR(&logger, format, ##__VA_ARGS__)
#define logc(format, ...) SPDLOG_LOGGER_CRITICAL(&logger, format, ##__VA_ARGS__)

int main()
{
    int valude  = 99;
    size_t size = 100;
    void *p     = &size;

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::trace);
    console_sink->set_pattern("%^%W %Y-%m-%d %H:%M:%S.%e %^%L %t %P [%s:%!:%#] %v%$");

    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("mylogs/multisink.txt", true);
    file_sink->set_level(spdlog::level::trace);
    file_sink->set_pattern("%^%W %Y-%m-%d %H:%M:%S.%e %^%L %t %P [%s:%!:%#] %v%$");

    spdlog::logger logger("multi_sink", {console_sink, file_sink});
    logger.set_level(spdlog::level::trace);

    logt("addr = {}, size = {}", p, size);
    logd("addr = {}, size = {}", p, size);
    logi("addr = {}, size = {}", p, size);
    logw("addr = {}, size = {}", p, size);
    loge("addr = {}, size = {}", p, size);
    logc("addr = {}, size = {}", p, size);

    logd("");
    logd("Easy padding in numbers like {:08d}", 12);
    logd("Support for int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);
    logd("Support for floats {:03.2f}", 1.23456);
    logd("Positional args are {1} {0}..", "too", "supported");
    logd("{:>8} aligned, {:<8} aligned", "right", "left");

    return 0;
}

/**
 g++ testSpdlog.cpp -I ../../3rdtools/include/
 ./a.out
 *
 */