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
#include "Protocol.hpp"

namespace easyrpc
{

class Client
{
public:
    Client() = default;
    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;

    Client(const std::string& ip, unsigned short port) 
        : m_socket(m_ioService), m_endpoint(boost::asio::ip::address::from_string(ip), port) {}
    ~Client()
    {
        stop();
    }

    void run()
    {
        m_thread = std::make_unique<std::thread>([this]{ m_ioService.run(); });
    }

    void stop()
    {
        disconnect();
        m_ioService.stop();
        if (m_thread != nullptr)
        {
            if (m_thread->joinable())
            {
                m_thread->join();
            }
        }
    }

    template<typename Protocol, typename... Args>
    typename std::enable_if<std::is_void<typename Protocol::ReturnType>::value, typename Protocol::ReturnType>::type
    call(const Protocol& protocol, Args&&... args)
    {
        connect();
        if (!write(protocol.name(), protocol.pack(std::forward<Args>(args)...)))
        {
            throw std::runtime_error("Write failed");
        }

        // 读取到buf后不进行任何处理，因为client建立的短连接
        // 需要server端进行确认后才能断开连接
        if (!read())
        {
            throw std::runtime_error("Read failed");
        }

        disconnect();
    }

    template<typename Protocol, typename... Args>
    typename std::enable_if<!std::is_void<typename Protocol::ReturnType>::value, typename Protocol::ReturnType>::type
    call(const Protocol& protocol, Args&&... args)
    {
        connect();
        if (!write(protocol.name(), protocol.pack(std::forward<Args>(args)...)))
        {
            throw std::runtime_error("Write failed");
        }

        if (!read())
        {
            throw std::runtime_error("Read failed");
        }

        disconnect();
        return protocol.unpack(std::string(&m_body[0], m_body.size()));
    }

private:
    void connect()
    {
        boost::system::error_code ec;
        m_socket.connect(m_endpoint, ec);
        if (ec)
        {
            throw std::runtime_error(ec.message()); 
        }
    }

    void disconnect()
    {
        m_socket.close();
    }

    bool write(const std::string& protocol, const std::string& body)
    {
        RequestHeader head { static_cast<unsigned int>(protocol.size()), static_cast<unsigned int>(body.size()) };
        if (head.protocolLen + head.bodyLen > MaxBufferLenght)
        {
            return false;
        }

        std::vector<boost::asio::const_buffer> buffer;
        buffer.emplace_back(boost::asio::buffer(&head, sizeof(RequestHeader)));
        buffer.emplace_back(boost::asio::buffer(protocol));
        buffer.emplace_back(boost::asio::buffer(body));

        boost::system::error_code ec;
        boost::asio::write(m_socket, buffer, ec);
        return ec ? false : true;
    }

    bool read()
    {
        boost::system::error_code ec;
        boost::asio::read(m_socket, boost::asio::buffer(m_head), ec);
        if (ec)
        {
            return false;
        }

        ResponseHeader head;
        memcpy(&head, m_head, sizeof(m_head));
        if (head.bodyLen <= 0 || head.bodyLen > MaxBufferLenght)
        {
            return false;
        }

        m_body.clear();
        m_body.resize(head.bodyLen);
        boost::asio::read(m_socket, boost::asio::buffer(m_body), ec); 
        return ec ? false : true;
    }

private:
    boost::asio::io_service m_ioService;
    boost::asio::ip::tcp::socket m_socket;
    boost::asio::ip::tcp::endpoint m_endpoint;
    std::unique_ptr<std::thread> m_thread;
    char m_head[ResponseHeaderLenght];
    std::vector<char> m_body;
};

}

#endif
