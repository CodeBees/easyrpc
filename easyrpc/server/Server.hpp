#ifndef _SERVER_H
#define _SERVER_H

#include "IoServicePool.hpp"
#include "Router.hpp"
#include "Connection.hpp"

namespace easyrpc
{

class Server
{
public:
    Server() = default;
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;
    Server(const std::string& ip, unsigned short port, std::size_t iosPoolSize, std::size_t timeoutMilli = 0)
        : m_iosPool(iosPoolSize), m_acceptor(m_iosPool.getIoService()), 
        m_ip(ip), m_port(port), m_timeoutMilli(timeoutMilli) {}

    ~Server()
    {
        stop();
    }

    void setThreadPoolSize(std::size_t size)
    {
        Router::instance().setThreadPoolSize(size);
    }

    void run()
    {
        bindAndListen();
        accept();
        m_iosPool.run();
    }

    void stop()
    {
        m_iosPool.stop();
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
    void bindAndListen()
    {
        boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address_v4::from_string(m_ip), m_port);
        m_acceptor.open(ep.protocol());
        m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        m_acceptor.bind(ep);
        m_acceptor.listen();
    }

    void accept()
    {
        std::shared_ptr<Connection> conn = std::make_shared<Connection>(m_iosPool.getIoService(), m_timeoutMilli);
        m_acceptor.async_accept(conn->socket(), [this, conn](boost::system::error_code ec)
        {
            if (!ec)
            {
                conn->start();
            }
            accept();
        });
    }

private:
    IoServicePool m_iosPool;
    boost::asio::ip::tcp::acceptor m_acceptor;
    std::string m_ip;
    unsigned short m_port = 0;
    std::size_t m_timeoutMilli = 0;
};

}

#endif
