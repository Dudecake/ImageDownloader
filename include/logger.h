#ifndef LOGGER_H
#define LOGGER_H

#include <memory>
#include <string>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_sinks.h>

namespace logger {
    inline auto getSpdLogger(const std::string &loggerName)
    {
        static spdlog::logger base = []{
            auto console_sink = std::make_shared<spdlog::sinks::stdout_sink_mt>();
            console_sink->set_level(spdlog::level::info);
            console_sink->set_pattern("%v");

            auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("/home/ckoomen/temp/image.log", true);
            file_sink->set_level(spdlog::level::debug);
            file_sink->set_pattern("%Y-%m-%d %T.%e %-5l %P --- [%14t] %-12n : %v");

            return spdlog::logger("temp", {console_sink, file_sink});
        }();
        base.set_level(spdlog::level::debug);

        return base.clone(loggerName);
}
}

#endif // LOGGER_H
