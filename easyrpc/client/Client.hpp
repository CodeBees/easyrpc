#ifndef _CLIENT_H
#define _CLIENT_H

#include <string>
#include <boost/asio.hpp>

namespace easyrpc
{

class Client
{
public:
    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;
    Client() : m_socket(m_ioService) {}

    void run()
    {
         
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

private:
    boost::asio::io_service m_ioService;
    boost::asio::ip::tcp::socket m_socket;
};

}

#endif
