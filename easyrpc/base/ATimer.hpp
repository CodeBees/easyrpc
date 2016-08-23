#ifndef _ATIMER_H
#define _ATIMER_H

#include <vector>
#include <atomic>
#include <functional>
#include <boost/timer.hpp>
#include <boost/asio.hpp>

template<typename Duration = boost::posix_time::milliseconds>
class ATimer
{
public:
    ATimer() = default;
    ATimer(boost::asio::io_service& ios) : m_timer(ios), m_isSingleShot(false) {}
    ~ATimer()
    {
        stop();
    }

    void start(unsigned int duration)
    {
        m_timer.expires_from_now(Duration(duration));
        m_timer.async_wait([this, duration](const boost::system::error_code& ec)
        {
            if (ec)
            {
                return;
            }

            for (auto& func : m_funcVec)
            {
                func();
            }

            if (!m_isSingleShot)
            {
                start(duration);
            }
        });
    }

    void stop()
    {
        m_timer.cancel();
    }

    void bind(const std::function<void()>& func)
    {
        m_funcVec.emplace_back(func);
    }

    void setSingleShot(bool isSingleShot)
    {
        m_isSingleShot = isSingleShot; 
    }

private:
    boost::asio::deadline_timer m_timer;
    std::function<void()> m_func = nullptr;
    std::vector<std::function<void()>> m_funcVec;
    std::atomic<bool> m_isSingleShot;
};

#endif
