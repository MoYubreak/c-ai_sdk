#include"../../include/until/mylog.h"
#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/async.h>
namespace ns_logger
{
    std::shared_ptr<spdlog::logger> Logger::_logger = nullptr;
    std::mutex Logger::_mutex;
    void Logger::initLogger(const std::string& loggerName , const std::string& logFile , spdlog::level::level_enum logLevel)
    {
        if(_logger == nullptr)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if(_logger == nullptr)
            {
                spdlog::flush_on(logLevel);
                spdlog::init_thread_pool(8192 , 1);

                if(logFile == "stdout")
                {
                    _logger = spdlog::stdout_color_mt(loggerName);
                }
                else
                {
                    _logger = spdlog::basic_logger_mt<spdlog::async_factory>(loggerName , logFile);
                }

                _logger->set_pattern("%H:%M [%n][%L] %v");
                _logger->set_level(logLevel);
            }
        }
    }
    std::shared_ptr<spdlog::logger> Logger::getLogger()
    {
        return _logger;
    }
    
}