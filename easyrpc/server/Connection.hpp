#ifndef _CONNECTION_H
#define _CONNECTION_H

#include <vector>
#include <memory>
#include <boost/asio.hpp>
#include <boost/timer.hpp>
#include "base/Header.hpp"
#include "base/ATimer.hpp"
#include "base/ScopeGuard.hpp"
#include "base/Logger.hpp"
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
    Connection(boost::asio::io_service& ios, std::size_t timeoutMilli = 0)
        : m_socket(ios), m_timer(ios), m_timeoutMilli(timeoutMilli) {}

    void start()
    {
        setNoDelay();
        readHead();
    }

    boost::asio::ip::tcp::socket& socket()
    {
        return m_socket;
    }

    void write(const std::string& body)
    {
        unsigned int bodyLen = static_cast<unsigned int>(body.size());
        if (bodyLen > MaxBufferLenght)
        {
            throw std::runtime_error("Send data too large");
        }

        const auto& buffer = getBuffer(ResponseHeader{ bodyLen }, body);
        writeImpl(buffer);
    }

    void disconnect()
    {
        if (m_socket.is_open())
        {
            boost::system::error_code ignoredec;
            m_socket.shutdown(boost::asio::socket_base::shutdown_both, ignoredec);
            m_socket.close(ignoredec);
        }
    }

private:
    void readHead()
    {
        startTimer();
        auto self(this->shared_from_this());
        boost::asio::async_read(m_socket, boost::asio::buffer(m_head), 
                                [this, self](boost::system::error_code ec, std::size_t)
        {
            auto guard = makeGuard([this, self]{ stopTimer(); disconnect(); });
            if (!m_socket.is_open())
            {
                logWarn("Socket is not open");
                return;
            }

            if (ec)
            {
                logWarn(ec.message());
                return;
            }

            RequestHeader head;
            memcpy(&head, m_head, sizeof(m_head));
            unsigned int len = head.protocolLen + head.bodyLen;
            if (len > 0 && len < MaxBufferLenght)
            {
                readProtocol(head);
                guard.dismiss();
                return;
            }
        });
    }

    void readProtocol(const RequestHeader& head)
    {
        m_protocol.clear();
        m_protocol.resize(head.protocolLen);
        auto self(this->shared_from_this());
        boost::asio::async_read(m_socket, boost::asio::buffer(m_protocol), 
                                [head, this, self](boost::system::error_code ec, std::size_t)
        {
            auto guard = makeGuard([this, self]{ stopTimer(); disconnect(); });
            if (!m_socket.is_open())
            {
                logWarn("Socket is not open");
                return;
            }

            if (ec)
            {
                logWarn(ec.message());
                return;
            }

            readBody(head.bodyLen, std::string(&m_protocol[0], m_protocol.size()));
            guard.dismiss();
        });
    }

    void readBody(unsigned int bodyLen, const std::string& protocol)
    {
        m_body.clear();
        m_body.resize(bodyLen);
        auto self(this->shared_from_this());
        boost::asio::async_read(m_socket, boost::asio::buffer(m_body), 
                                [protocol, this, self](boost::system::error_code ec, std::size_t)
        {
            stopTimer();
            if (!m_socket.is_open())
            {
                logWarn("Socket is not open");
                return;
            }

            if (ec)
            {
                logWarn(ec.message());
                disconnect();
                return;
            }
                              
            Router::instance().route(protocol, std::string(&m_body[0], m_body.size()), self);
        });
    }

    void setNoDelay()
    {
        boost::asio::ip::tcp::no_delay option(true);
        boost::system::error_code ec;
        m_socket.set_option(option, ec);
    }

    void startTimer()
    {
        if (m_timeoutMilli == 0)
        {
            return;
        }

        auto self(this->shared_from_this());
        m_timer.bind([this, self]{ disconnect(); });
        m_timer.setSingleShot(true);
        m_timer.start(m_timeoutMilli);
    }

    void stopTimer()
    {
        if (m_timeoutMilli == 0)
        {
            return;
        }
        m_timer.stop();
    }

    std::vector<boost::asio::const_buffer> getBuffer(const ResponseHeader& head, const std::string& body)
    {
        std::vector<boost::asio::const_buffer> buffer;
        buffer.emplace_back(boost::asio::buffer(&head, sizeof(ResponseHeader)));
        buffer.emplace_back(boost::asio::buffer(body));
        return buffer;
    }

    void writeImpl(const std::vector<boost::asio::const_buffer>& buffer)
    {
        boost::system::error_code ec;
        boost::asio::write(m_socket, buffer, ec);
        if (ec)
        {
            throw std::runtime_error(ec.message());
        }
    }

private:
    boost::asio::ip::tcp::socket m_socket;
    char m_head[RequestHeaderLenght];
    std::vector<char> m_protocol;
    std::vector<char> m_body;
    ATimer<> m_timer;
    std::size_t m_timeoutMilli = 0;
};

}

#endif
