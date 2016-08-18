#ifndef _IOSERVERPOOL_H
#define _IOSERVERPOOL_H

#include <vector>
#include <memory>
#include <thread>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

namespace easyrpc
{

class IoServicePool
{
public:
    IoServicePool() = default;
    IoServicePool(IoServicePool&) = delete;
    IoServicePool& operator=(const IoServicePool&) = delete;

    explicit IoServicePool(std::size_t poolSize)
    {
        if (poolSize == 0)
        {
            throw std::runtime_error("io server pool size is 0");
        }
        
        for (std::size_t i = 0; i < poolSize; ++i)
        {
            IoServerPtr ioService = std::make_shared<boost::asio::io_service>();
            WorkPtr work = std::make_shared<boost::asio::io_service::work>(*ioService);
            m_ioServiceVec.emplace_back(ioService);
            m_workVec.emplace_back(work);
        }
    }

    void run()
    {
        for (std::size_t i = 0; i < m_ioServiceVec.size(); ++i)
        {
            std::shared_ptr<std::thread> t = 
                std::make_shared<std::thread>(boost::bind(&boost::asio::io_service::run, m_ioServiceVec[i]));
            m_threadVec.emplace_back(t);
        }
    }

    void stop()
    {
        for (auto& iter : m_ioServiceVec)
        {
            iter->stop();
        }

        for (auto& iter : m_threadVec)
        {
            if (iter->joinable())
            {
                iter->join();
            }
        }
    }

    boost::asio::io_service& getIoService()
    {
        boost::asio::io_service& ioService = *m_ioServiceVec[m_nextIoService];
        ++m_nextIoService;
        if (m_nextIoService == m_ioServiceVec.size())
        {
            m_nextIoService = 0;
        }
        return ioService;
    }

private:
    using IoServerPtr = std::shared_ptr<boost::asio::io_service>;
    using WorkPtr = std::shared_ptr<boost::asio::io_service::work>;
    std::vector<IoServerPtr> m_ioServiceVec;
    std::vector<WorkPtr> m_workVec;
    std::vector<std::shared_ptr<std::thread>> m_threadVec; 
    std::size_t m_nextIoService = 0;
};

}

#endif
