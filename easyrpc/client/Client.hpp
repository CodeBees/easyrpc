#ifndef _CLIENT_H
#define _CLIENT_H

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <thread>
#include <memory>
#include <iostream>
#include <boost/asio.hpp>
#include "base/Header.hpp"
#include "base/ATimer.hpp"
#include "base/ScopeGuard.hpp"
#include "base/StringUtil.hpp"
#include "Protocol.hpp"

namespace easyrpc
{

class Client
{
public:
    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;
    Client() : m_work(m_ios), m_socket(m_ios), 
    m_timerWork(m_timerIos), m_timer(m_timerIos) {}

    ~Client()
    {
        stop();
    }

    Client& connect(const std::string& address)
    {
        std::vector<std::string> token = StringUtil::split(address, ":");
        if (token.size() != 2)
        {
            throw std::invalid_argument("Address format error");
        }
        return connect(token[0], token[1]);
    }

    Client& connect(const std::string& ip, unsigned short port)
    {
        return connect(ip, std::to_string(port));
    }

    Client& connect(const std::string& ip, const std::string& port)
    {
        boost::asio::ip::tcp::resolver resolver(m_ios);
        boost::asio::ip::tcp::resolver::query query(boost::asio::ip::tcp::v4(), ip, port);
        m_endpointIter = resolver.resolve(query);
        return *this;
    }

    Client& timeout(std::size_t timeoutMilli)
    {
        m_timeoutMilli = timeoutMilli;
        return *this;
    }

    void run()
    {
        m_thread = std::make_unique<std::thread>([this]{ m_ios.run(); });
        if (m_timeoutMilli != 0)
        {
            m_timerThread = std::make_unique<std::thread>([this]{ m_timerIos.run(); });
        }
    }

    void stop()
    {
        stopIosThread();
        stopTimerThread();
    }

    template<typename Protocol, typename... Args>
    typename std::enable_if<std::is_void<typename Protocol::ReturnType>::value, typename Protocol::ReturnType>::type
    call(const Protocol& protocol, Args&&... args)
    {
        connect();
        auto guard = makeGuard([this]{ disconnect(); });
        // 读取到buf后不进行任何处理，因为client建立的短连接.
        // 需要server端进行确认后才能断开连接.
        callImpl(protocol.name(), protocol.pack(std::forward<Args>(args)...));
    }

    template<typename Protocol, typename... Args>
    typename std::enable_if<!std::is_void<typename Protocol::ReturnType>::value, typename Protocol::ReturnType>::type
    call(const Protocol& protocol, Args&&... args)
    {
        connect();
        auto guard = makeGuard([this]{ disconnect(); });
        callImpl(protocol.name(), protocol.pack(std::forward<Args>(args)...));
        return protocol.unpack(std::string(&m_body[0], m_body.size()));
    }

private:
    void connect()
    {
        boost::asio::connect(m_socket, m_endpointIter);
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

    void callImpl(const std::string& protocol, const std::string& body)
    {
        write(protocol, body);
        read();
    }

    void write(const std::string& protocol, const std::string& body)
    {
        unsigned int protocolLen = static_cast<unsigned int>(protocol.size());
        unsigned int bodyLen = static_cast<unsigned int>(body.size());
        if (protocolLen + bodyLen > MaxBufferLenght)
        {
            throw std::runtime_error("Send data too large");
        }

        auto buffer = getBuffer(RequestHeader{ protocolLen, bodyLen }, protocol, body);
        writeImpl(buffer);
    }

    std::vector<boost::asio::const_buffer> getBuffer(const RequestHeader& head, 
                                                     const std::string& protocol, 
                                                     const std::string& body)
    {
        std::vector<boost::asio::const_buffer> buffer;
        buffer.emplace_back(boost::asio::buffer(&head, sizeof(RequestHeader)));
        buffer.emplace_back(boost::asio::buffer(protocol));
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

    void read()
    {
        startTimer();
        auto guard = makeGuard([this]{ stopTimer(); });
        readHead();
        checkHead();
        readBody();
    }

    void readHead()
    {
        boost::system::error_code ec;
        boost::asio::read(m_socket, boost::asio::buffer(m_head), ec);
        if (ec)
        {
            throw std::runtime_error(ec.message());
        }
    }

    void checkHead()
    {
        memcpy(&m_resHead, m_head, sizeof(m_head));
        if (m_resHead.bodyLen <= 0 || m_resHead.bodyLen > MaxBufferLenght)
        {
            throw std::runtime_error("Invaild bodylen");
        }
    }

    void readBody()
    {
        m_body.clear();
        m_body.resize(m_resHead.bodyLen);
        boost::system::error_code ec;
        boost::asio::read(m_socket, boost::asio::buffer(m_body), ec); 
        if (ec)
        {
            throw std::runtime_error(ec.message());
        }
    }

    void startTimer()
    {
        if (m_timeoutMilli == 0)
        {
            return;
        }

        m_timer.bind([this]{ disconnect(); });
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

    void stopIosThread()
    {
        m_ios.stop();
        if (m_thread != nullptr)
        {
            if (m_thread->joinable())
            {
                m_thread->join();
            }
        }
    }

    void stopTimerThread()
    {
        m_timerIos.stop();
        if (m_timerThread != nullptr)
        {
            if (m_timerThread->joinable())
            {
                m_timerThread->join();
            }
        }
    }

private:
    boost::asio::io_service m_ios;
    boost::asio::io_service::work m_work;
    boost::asio::ip::tcp::socket m_socket;
    boost::asio::ip::tcp::resolver::iterator m_endpointIter;
    std::unique_ptr<std::thread> m_thread;
    char m_head[ResponseHeaderLenght];
    ResponseHeader m_resHead;
    std::vector<char> m_body;
    boost::asio::io_service m_timerIos;
    boost::asio::io_service::work m_timerWork;
    std::unique_ptr<std::thread> m_timerThread;
    ATimer<> m_timer;
    std::size_t m_timeoutMilli = 0;
};

}

#endif
