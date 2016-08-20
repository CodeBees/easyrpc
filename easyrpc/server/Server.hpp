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
    Server(const std::string& ip, unsigned short port, std::size_t ioPoolSize, std::size_t timeoutMilli = 0)
        : m_ioServicePool(ioPoolSize), m_acceptor(m_ioServicePool.getIoService()), 
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
        m_ioServicePool.run();
    }

    void stop()
    {
        m_ioServicePool.stop();
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
        /* boost::asio::ip::tcp::endpoint ep(boost::asio::ip::tcp::v4(), m_port); */
        boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address_v4::from_string(m_ip), m_port);
        m_acceptor.open(ep.protocol());
        m_acceptor.bind(ep);
        m_acceptor.listen();
    }

    void accept()
    {
        m_conn = std::make_shared<Connection>(m_ioServicePool.getIoService(), m_timeoutMilli);
        m_acceptor.async_accept(m_conn->socket(), [this](boost::system::error_code ec)
        {
            if (!ec)
            {
                m_conn->start();
            }
            accept();
        });
    }

private:
    IoServicePool m_ioServicePool;
    boost::asio::ip::tcp::acceptor m_acceptor;
    std::shared_ptr<Connection> m_conn;
    std::string m_ip = "127.0.0.1";
    unsigned short m_port = 8888;
    std::size_t m_timeoutMilli = 0;
};

}

#endif
