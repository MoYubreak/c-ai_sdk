#pragma once
#include<memory>
#include<spdlog/spdlog.h>
#include <spdlog/logger.h>
#include<mutex>
#include<string>
namespace ns_logger
{
    
    class Logger
    {
    public:
        static void initLogger(const std::string& loggerName , const std::string& logFile , spdlog::level::level_enum logLevel = spdlog::level::level_enum::info);
        static std::shared_ptr<spdlog::logger> getLogger();
    private:
        Logger() = default;
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;
    private:
        static std::shared_ptr<spdlog::logger> _logger;
        static std::mutex _mutex;
    };
    #define TRACE(format,...) ns_logger::Logger::getLogger()->trace(std::string("[{\:>10s}\:{\:<4d}]") + format , __FILE__ , __LINE__ , ##__VA_ARGS__)
    #define INFO(format,...) ns_logger::Logger::getLogger()->info(std::string("[{\:>10s}\:{\:<4d}]") + format , __FILE__ , __LINE__ , ##__VA_ARGS__)
    #define DBG(format,...) ns_logger::Logger::getLogger()->debug(std::string("[{\:>10s}\:{\:<4d}]") + format , __FILE__ , __LINE__ , ##__VA_ARGS__)
    #define WARN(format,...) ns_logger::Logger::getLogger()->warn(std::string("[{\:>10s}\:{\:<4d}]") + format , __FILE__ , __LINE__ , ##__VA_ARGS__)
    #define ERR(format,...) ns_logger::Logger::getLogger()->error(std::string("[{\:>10s}\:{\:<4d}]") + format , __FILE__ , __LINE__ , ##__VA_ARGS__)
    #define CRI(format,...) ns_logger::Logger::getLogger()->critical(std::string("[{\:>10s}\:{\:<4d}]") + format , __FILE__ , __LINE__ , ##__VA_ARGS__)
}