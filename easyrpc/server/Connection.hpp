#ifndef _CONNECTION_H
#define _CONNECTION_H

#include <iostream>
#include <vector>
#include <memory>
#include <boost/asio.hpp>
#include <boost/timer.hpp>
#include "base/Header.hpp"
#include "Router.hpp"

namespace easyrpc
{

class Connection : public std::enable_shared_from_this<Connection>
{
public:
    friend class Router;
    Connection() = default;
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;
    Connection(boost::asio::io_service& ioService, std::size_t timeoutMilli = 0)
        : m_socket(ioService), m_timer(ioService), m_timeoutMilli(timeoutMilli) {}

    void start()
    {
        setNoDelay();
        readHead();
    }

    boost::asio::ip::tcp::socket& socket()
    {
        return m_socket;
    }

    bool write(const std::string& body)
    {
        ResponseHeader head { static_cast<unsigned int>(body.size()) };
        if (head.bodyLen > MaxBufferLenght)
        {
            return false;
        }

        std::vector<boost::asio::const_buffer> buffer;
        buffer.emplace_back(boost::asio::buffer(&head, sizeof(ResponseHeader)));
        buffer.emplace_back(boost::asio::buffer(body));

        boost::system::error_code ec;
        boost::asio::write(m_socket, buffer, ec);
        std::cout << "write success" << std::endl;
        return ec ? false : true;
    }

private:
    void readHead()
    {
        resetTimer();
        auto self(this->shared_from_this());
        boost::asio::async_read(m_socket, boost::asio::buffer(m_head), [this, self](boost::system::error_code ec, std::size_t)
        {
            if (!m_socket.is_open())
            {
                cancelTimer();
                return;
            }

            if (ec)
            {
                std::cout << "Error: " << ec.message()  << ", line: " << __LINE__ << std::endl;
                cancelTimer();
                close();
                return;
            }

            RequestHeader head;
            memcpy(&head, m_head, sizeof(m_head));
            unsigned int len = head.protocolLen + head.bodyLen;
            if (len > 0 && len < MaxBufferLenght)
            {
                readProtocol(head);
                return;
            }
            else
            {
                cancelTimer();
                close();
            }

#if 0
            if (len == 0)
            {
                readHead();
            }
            else
            {
                cancelTimer();
            }
#endif
        });
    }

    void readProtocol(const RequestHeader& head)
    {
        m_protocol.clear();
        m_protocol.resize(head.protocolLen);
        auto self(this->shared_from_this());
        boost::asio::async_read(m_socket, boost::asio::buffer(m_protocol), [&head, this, self](boost::system::error_code ec, std::size_t)
        {
            if (!m_socket.is_open())
            {
                cancelTimer();
                return;
            }

            if (ec)
            {
                std::cout << "Error: " << ec.message()  << ", line: " << __LINE__ << std::endl;
                cancelTimer();
                close();
                return;
            }

            readBody(head.bodyLen, std::string(&m_protocol[0], m_protocol.size()));
        });
    }

    void readBody(unsigned int bodyLen, const std::string& protocol)
    {
        m_body.clear();
        m_body.resize(bodyLen);
        auto self(this->shared_from_this());
        boost::asio::async_read(m_socket, boost::asio::buffer(m_body), [&protocol, this, self](boost::system::error_code ec, std::size_t)
        {
            cancelTimer();
            if (!m_socket.is_open())
            {
                return;
            }

            if (ec)
            {
                std::cout << "Error: " << ec.message()  << ", line: " << __LINE__ << std::endl;
                close();
                return;
            }
                              
            /* readHead(); */
            Router::instance().route(protocol, std::string(&m_body[0], m_body.size()), self);
        });
    }

    void setNoDelay()
    {
        boost::asio::ip::tcp::no_delay option(true);
        boost::system::error_code ec;
        m_socket.set_option(option, ec);
    }

    void close()
    {
        boost::system::error_code ignoredec;
        m_socket.close(ignoredec);
    }

    void resetTimer()
    {
        if (m_timeoutMilli == 0)
        {
            return;
        }

        auto self(this->shared_from_this());
        m_timer.expires_from_now(boost::posix_time::milliseconds(m_timeoutMilli));
        m_timer.async_wait([this, self](const boost::system::error_code& ec)
        {
            if (!m_socket.is_open())
            {
                return;
            }

            if (ec)
            {
                std::cout << "Error: " << ec.message()  << ", line: " << __LINE__ << std::endl;
                return;
            }

            // Connection timeout
            close();
        });
    }

    void cancelTimer()
    {
        if (m_timeoutMilli == 0)
        {
            return;
        }
        m_timer.cancel();
    }

private:
    boost::asio::ip::tcp::socket m_socket;
    boost::asio::deadline_timer m_timer;
    char m_head[RequestHeaderLenght];
    std::vector<char> m_protocol;
    std::vector<char> m_body;
    std::size_t m_timeoutMilli = 0;
};

}

#endif
