#ifndef _SERVER_H
#define _SERVER_H

#include "IoServicePool.hpp"
#include "Router.hpp"

namespace easyrpc
{

class Server
{
public:
    Server() = default;
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    void setThreadPoolSize(std::size_t size)
    {
        Router::instance().setThreadPoolSize(size);
    }

    void route(const std::string& funcName, const std::string& text)
    {
        Router::instance().route(funcName, text);
    }

    template<typename Function>
    void bind(const std::string& funcName, const Function& func)
    {
        Router::instance().bind(funcName, func);
    }

    template<typename Function, typename Self>
    void bind(const std::string& funcName, const Function& func, Self* self)
    {
        Router::instance().bind(funcName, func, self); 
    }

private:
    IoServicePool m_ioServicePool;
};

}

#endif
