#ifndef _CLIENT_H
#define _CLIENT_H

#include <string>
#include <vector>
#include <array>
#include <thread>
#include <memory>
#include <boost/asio.hpp>
#include "base/Header.hpp"
#include "Protocol.hpp"

namespace easyrpc
{

class Client
{
public:
    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;
    Client() : m_socket(m_ioService) {}
    ~Client()
    {
        stop();
    }

    void run()
    {
        m_thread = std::make_unique<std::thread>([this]{ m_ioService.run(); });
    }

    void connect(const std::string& ip, unsigned short port)
    {
        boost::asio::ip::tcp::resolver resolver(m_ioService);
        boost::asio::ip::tcp::resolver::query query(boost::asio::ip::tcp::v4(), ip, std::to_string(port));
        boost::asio::ip::tcp::resolver::iterator iter = resolver.resolve(query);
        boost::asio::connect(m_socket, iter);
    }

    void disconnect()
    {
        m_socket.shutdown(boost::asio::socket_base::shutdown_both); 
        m_socket.close();
    }

    void stop()
    {
        disconnect();
        m_ioService.stop();
        if (m_thread->joinable())
        {
            m_thread->join();
        }
    }

    template<typename Protocol, typename... Args>
    typename std::enable_if<std::is_void<typename Protocol::ReturnType>::value>::type 
    call(const Protocol& protocol, Args&&... args)
    {
        if (!write(protocol.funcName(), protocol.pack(std::forward<Args>(args)...)))
        {
            throw std::runtime_error("Write failed");
        }
    }

    template<typename Protocol, typename... Args>
    typename std::enable_if<!std::is_void<typename Protocol::ReturnType>::value, typename Protocol::ReturnType>::type
    call(const Protocol& protocol, Args&&... args)
    {
        if (!write(protocol.funcName(), protocol.pack(std::forward<Args>(args)...)))
        {
            throw std::runtime_error("Write failed");
        }

        if (!read())
        {
            throw std::runtime_error("Read failed");
        }

        return protocol.unpack(m_body.data());
    }

private:
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

        /* ResponseHeader head = *(ResponseHeader*)m_head; */
        ResponseHeader head;
        memcpy(&head, m_head, sizeof(m_head));
        if (head.bodyLen <= 0 || head.bodyLen > MaxBufferLenght)
        {
            return false;
        }

        boost::asio::read(m_socket, boost::asio::buffer(m_body.data(), head.bodyLen), ec); 
        return ec ? false : true;
    }

private:
    boost::asio::io_service m_ioService;
    boost::asio::ip::tcp::socket m_socket;
    std::unique_ptr<std::thread> m_thread;
    char m_head[RequestHeaderLenght];
    std::array<char, MaxBufferLenght> m_body;
};

}

#endif
