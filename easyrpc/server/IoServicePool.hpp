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
            m_iosVec.emplace_back(ioService);
            m_workVec.emplace_back(work);
        }
    }

    void run()
    {
        for (std::size_t i = 0; i < m_iosVec.size(); ++i)
        {
            std::shared_ptr<std::thread> t = 
                std::make_shared<std::thread>(boost::bind(&boost::asio::io_service::run, m_iosVec[i]));
            m_threadVec.emplace_back(t);
        }
    }

    void stop()
    {
        stopIoServices();
        stopThreads();
    }

    boost::asio::io_service& getIoService()
    {
        boost::asio::io_service& ioService = *m_iosVec[m_nextIoService];
        ++m_nextIoService;
        if (m_nextIoService == m_iosVec.size())
        {
            m_nextIoService = 0;
        }
        return ioService;
    }

private:
    void stopIoServices()
    {
        for (auto& iter : m_iosVec)
        {
            if (iter != nullptr)
            {
                iter->stop();
            }
        }
    }

    void stopThreads()
    {
        for (auto& iter : m_threadVec)
        {
            if (iter != nullptr)
            {
                if (iter->joinable())
                {
                    iter->join();
                }
            }
        }       
    }

private:
    using IoServerPtr = std::shared_ptr<boost::asio::io_service>;
    using WorkPtr = std::shared_ptr<boost::asio::io_service::work>;
    std::vector<IoServerPtr> m_iosVec;
    std::vector<WorkPtr> m_workVec;
    std::vector<std::shared_ptr<std::thread>> m_threadVec; 
    std::size_t m_nextIoService = 0;
};

}

#endif
