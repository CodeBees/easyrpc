#ifndef _LOGGER_H
#define _LOGGER_H

#include "spdlog/spdlog.h"

static const std::string LoggerName = "easyrpc";
static const std::size_t MaxFileSize = 3 * 1024 * 1024;
static const std::size_t MaxFiles = 30;

class LoggerImpl
{
public:
    LoggerImpl(const LoggerImpl&) = delete;
    LoggerImpl& operator=(const LoggerImpl&) = delete;
    LoggerImpl()
    {
        init();
    }

    static LoggerImpl& instance()
    {
        static LoggerImpl logger;
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
            m_fileLogger = spdlog::rotating_logger_mt("file", LoggerName, MaxFileSize, MaxFiles, true);
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

class Logger
{
public:
    Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(const char* filePath, const char* funcName, unsigned long line, spdlog::level::level_enum level)
        : m_filePath(filePath), m_funcName(funcName), m_line(line), m_level(level) {}

    template<typename... Args>
    void log(const std::string& fmt, Args&&... args)
    {
        log(fmt.c_str(), std::forward<Args>(args)...);
    }

    template<typename... Args>
    void log(const char* fmt, Args&&... args)
    {
        const std::string& content = makeContent(fmt);
        LoggerImpl::instance().getConsoleLogger()->log(m_level, content.c_str(), std::forward<Args>(args)...);
        LoggerImpl::instance().getFileLogger()->log(m_level, content.c_str(), std::forward<Args>(args)...);
    }

private:
    std::string makeContent(const std::string& fmt)
    {
        int pos = m_filePath.find_last_of("/");
        std::string content = m_filePath.substr(pos + 1) + " " + m_funcName + "(" + std::to_string(m_line) + ") " + fmt;
        return content;
    }

private:
    std::string m_filePath;
    std::string m_funcName;
    unsigned long m_line;
    spdlog::level::level_enum m_level;
};

#define LOCATION       __FILE__, __FUNCTION__, __LINE__
#define logTrace       Logger(LOCATION, spdlog::level::level_enum::trace).log
#define logDebug       Logger(LOCATION, spdlog::level::level_enum::debug).log
#define logInfo        Logger(LOCATION, spdlog::level::level_enum::info).log
#define logWarn        Logger(LOCATION, spdlog::level::level_enum::warn).log
#define logError       Logger(LOCATION, spdlog::level::level_enum::err).log
#define logCritical    Logger(LOCATION, spdlog::level::level_enum::critical).log

#endif
