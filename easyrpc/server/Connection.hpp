#ifndef _CONNECTION_H
#define _CONNECTION_H

#include <memory>
#include <boost/asio.hpp>
#include <boost/timer.hpp>
#include "base/Header.hpp"

namespace easyrpc
{

class Connection : public std::enable_shared_from_this<Connection>
{
public:
    Connection() = default;
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    void start()
    {
        setNoDelay();
        readHead();
    }

    boost::asio::ip::tcp::socket& socket()
    {
        return m_socket;
    }

private:
    void readHead()
    {
        auto self(this->shared_from_this());
        boost::asio::async_read(m_socket, boost::asio::buffer(m_head), [this, self](boost::system::error_code& ec, std::size_t)
        {
            if (!m_socket.is_open() || ec)
            {
                return;
            }

            RequestHeader head;
            memcpy(&head, m_head, sizeof(m_head));
            unsigned int len = head.protocolLen + head.bodyLen;
            if (len > 0 && len < MaxBufferLenght)
            {
                readBody();
                return;
            }

            if (len == 0)
            {
                readHead();
            }
        });
    }

    void readProtocol()
    {

    }

    void readBody()
    {
        
    }

    void setNoDelay()
    {
        boost::asio::ip::tcp::no_delay option(true);
        boost::system::error_code ec;
        m_socket.set_option(option, ec);
    }

private:
    boost::asio::ip::tcp::socket m_socket;
    char m_head[RequestHeaderLenght];
    std::size_t m_timeOutMilli = 0;
};

}

#endif
