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
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;
    Server() : m_iosPool(std::thread::hardware_concurrency()), 
    m_acceptor(m_iosPool.getIoService()) {}

    ~Server()
    {
        stop();
    }

    Server& listen(const std::string& port)
    {
        return listen("0.0.0.0", port);    
    }

    Server& listen(unsigned short port)
    {
        return listen("0.0.0.0", port);
    }

    Server& listen(const std::string& ip, const std::string& port)
    {
        return listen(ip, static_cast<unsigned short>(std::stoi(port)));
    }

    Server& listen(const std::string& ip, unsigned short port)
    {
        m_ip = ip;
        m_port = port;
        return *this;
    }

    Server& timeout(std::size_t timeoutMilli)
    {
        m_timeoutMilli = timeoutMilli;
        return *this;
    }

    Server& multithreaded(std::size_t num)
    {
        m_threadNum = num;
        return *this;
    }

    void run()
    {
        Router::instance().multithreaded(m_threadNum);
        listen();
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

    void unbind(const std::string& funcName)
    {
        Router::instance().unbind(funcName);
    }

private:
    void listen()
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
    std::string m_ip = "0.0.0.0";
    unsigned short m_port = 50051;
    std::size_t m_timeoutMilli = 0;
    std::size_t m_threadNum = 1;
};

}

#endif
