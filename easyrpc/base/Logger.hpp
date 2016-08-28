#ifndef _LOGGER_H
#define _LOGGER_H

#include <memory>
#include "spdlog/spdlog.h"

class Logger
{
public:
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger()
    {
        init();
    }

    static Logger& instance()
    {
        static Logger logger;
        return logger;
    }

    std::shared_ptr<spdlog::logger> getConsoleLogger()
    {
        return m_consoleLogger;
    }

    std::shared_ptr<spdlog::logger> getFileLogger()
    {
        return m_fileLogger;
    }

private:
    bool init()
    {
        try
        {
            m_consoleLogger = spdlog::stdout_logger_mt("console", true);
            m_fileLogger = spdlog::rotating_logger_mt("logger", "text.log", 1024 * 1024 * 3, 10, true);
            m_consoleLogger->set_level(spdlog::level::debug); 
            m_fileLogger->set_level(spdlog::level::debug); 
        }
        catch (std::exception&)
        {
            return false;
        }

        return true;
    }

private:
    std::shared_ptr<spdlog::logger> m_consoleLogger;
    std::shared_ptr<spdlog::logger> m_fileLogger;
};

template<typename... Args>
static void logDebug(const char* fmt, const Args&... args)
{
    Logger::instance().getConsoleLogger()->debug(fmt, args...);
    Logger::instance().getFileLogger()->debug(fmt, args...);
}

template<typename... Args>
static void logDebug(const std::string& fmt, const Args&... args)
{
    Logger::instance().getConsoleLogger()->debug(fmt.c_str(), args...);
    Logger::instance().getFileLogger()->debug(fmt.c_str(), args...);
}

template<typename... Args>
static void logInfo(const char* fmt, Args&&... args)
{
    Logger::instance().getConsoleLogger()->info(fmt, std::forward<Args>(args)...);
    Logger::instance().getFileLogger()->info(fmt, std::forward<Args>(args)...);
}

template<typename... Args>
static void logInfo(const std::string& fmt, Args&&... args)
{
    Logger::instance().getConsoleLogger()->info(fmt.c_str(), std::forward<Args>(args)...);
    Logger::instance().getFileLogger()->info(fmt.c_str(), std::forward<Args>(args)...);
}

template<typename... Args>
static void logWarn(const char* fmt, Args&&... args)
{
    Logger::instance().getConsoleLogger()->warn(fmt, std::forward<Args>(args)...);
    Logger::instance().getFileLogger()->warn(fmt, std::forward<Args>(args)...);
}

template<typename... Args>
static void logWarn(const std::string& fmt, Args&&... args)
{
    Logger::instance().getConsoleLogger()->warn(fmt.c_str(), std::forward<Args>(args)...);
    Logger::instance().getFileLogger()->warn(fmt.c_str(), std::forward<Args>(args)...);
}

template<typename... Args>
static void logError(const char* fmt, Args&&... args)
{
    Logger::instance().getConsoleLogger()->error(fmt, std::forward<Args>(args)...);
    Logger::instance().getFileLogger()->error(fmt, std::forward<Args>(args)...);
}

template<typename... Args>
static void logError(const std::string& fmt, Args&&... args)
{
    Logger::instance().getConsoleLogger()->error(fmt.c_str(), std::forward<Args>(args)...);
    Logger::instance().getFileLogger()->error(fmt.c_str(), std::forward<Args>(args)...);
}

template<typename... Args>
static void logCritical(const char* fmt, Args&&... args)
{
    Logger::instance().getConsoleLogger()->critical(fmt, std::forward<Args>(args)...);
    Logger::instance().getFileLogger()->critical(fmt, std::forward<Args>(args)...);
}

template<typename... Args>
static void logCritical(const std::string& fmt, Args&&... args)
{
    Logger::instance().getConsoleLogger()->critical(fmt.c_str(), std::forward<Args>(args)...);
    Logger::instance().getFileLogger()->critical(fmt.c_str(), std::forward<Args>(args)...);
}

#endif
