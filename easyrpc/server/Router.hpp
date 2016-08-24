#ifndef _ROUTER_H
#define _ROUTER_H

#include <iostream>
#include <unordered_map>
#include <map>
#include <tuple>
#include <type_traits>
#include "base/FunctionTraits.hpp"
#include "base/ThreadPool.hpp"
#include "TokenParser.hpp"

namespace easyrpc
{

class InvokerFunction
{
public:
    using Function = std::function<void(TokenParser& parser, std::string& result)>;
    InvokerFunction() = default;
    InvokerFunction(const Function& func, std::size_t paramSize) : m_func(func), m_paramSize(paramSize) {}

    template<typename T>
    void operator()(const std::string& body, T conn)
    {
        if (!body.empty())
        {
            try
            {
                TokenParser parser(body);
                std::string result;
                m_func(parser, result);
                if (!result.empty())
                {
                    if (!conn->write(result))
                    {
                        std::cout << "Write failed" << std::endl;
                        conn->disconnect();
                    }
                }
            }
            catch (std::exception& e)
            {
                std::cout << "Exception: " << e.what() << std::endl;
            }
        }
    }

    std::size_t paramSize() const
    {
        return m_paramSize;
    }

private:
    Function m_func = nullptr;
    std::size_t m_paramSize = 0;
};

class Router
{
public:
    Router() = default;
    Router(const Router&) = delete;
    Router& operator=(const Router&) = delete;
    ~Router()
    {
        stop();
    }

    static Router& instance()
    {
        static Router router;
        return router;
    }

    void setThreadPoolSize(std::size_t size)
    {
        m_threadPool.initThreadNum(size);
    }

    void stop()
    {
        m_threadPool.stop();
    }

    template<typename Function>
    void bind(const std::string& protocol, const Function& func)
    {
        bindNonMemberFunc(protocol, func);
    }

    template<typename Function, typename Self>
    void bind(const std::string& protocol, const Function& func, Self* self)
    {
        bindMemberFunc(protocol, func, self); 
    }

    template<typename T>
    void route(const std::string& protocol, const std::string& body, T conn)
    {
        auto iter = m_invokerMap.find(protocol);
        if (iter == m_invokerMap.end())
        {
            return;
        }

        m_threadPool.addTask(iter->second, body, conn);
    }

private:
    template<typename Function, typename... Args>
    static typename std::enable_if<std::is_void<typename std::result_of<Function(Args...)>::type>::value>::type
    call(const Function& func, const std::tuple<Args...>& tp, std::string& result)
    {
        callImpl(func, std::make_index_sequence<sizeof...(Args)>{}, tp);
        result = pack(true);
    }

    template<typename Function, typename... Args>
    static typename std::enable_if<!std::is_void<typename std::result_of<Function(Args...)>::type>::value>::type
    call(const Function& func, const std::tuple<Args...>& tp, std::string& result)
    {
        auto ret = callImpl(func, std::make_index_sequence<sizeof...(Args)>{}, tp);
        // 将ret序列化放入result
        result = pack(ret);
    }

    template<typename Function, std::size_t... I, typename... Args>
    static auto callImpl(const Function& func, const std::index_sequence<I...>&, const std::tuple<Args...>& tp)
    {
        return func(std::get<I>(tp)...);
    }

    template<typename Function, typename Self, typename... Args>
    static typename std::enable_if<std::is_void<typename std::result_of<Function(Self, Args...)>::type>::value>::type
    callMember(const Function& func, Self* self, const std::tuple<Args...>& tp, std::string& result)
    {
        callMemberImpl(func, self, std::make_index_sequence<sizeof...(Args)>{}, tp);
        result = pack(true);
    }

    template<typename Function, typename Self, typename... Args>
    static typename std::enable_if<!std::is_void<typename std::result_of<Function(Self, Args...)>::type>::value>::type
    callMember(const Function& func, Self* self, const std::tuple<Args...>& tp, std::string& result)
    {
        auto ret = callMemberImpl(func, self, std::make_index_sequence<sizeof...(Args)>{}, tp);
        // 将ret序列化放入result
        result = pack(ret);
    }

    template<typename Function, typename Self, std::size_t... I, typename... Args>
    static auto callMemberImpl(const Function& func, Self* self, const std::index_sequence<I...>&, const std::tuple<Args...>& tp)
    {
        return (*self.*func)(std::get<I>(tp)...);
    }

private:
    // 遍历function的实参类型，将TokenParser解析出来的参数转换为实参并添加到std::tuple中.
    template<typename Function, std::size_t I = 0, std::size_t N = FunctionTraits<Function>::arity>
    class Invoker
    {
    public:
        template<typename Args>
        static void apply(const Function& func, const Args& args, TokenParser& parser, std::string& result)
        {
            using ArgType = typename FunctionTraits<Function>::template args<I>::type;
            try
            {
                Invoker<Function, I + 1, N>::apply(func, std::tuple_cat(args, std::make_tuple(parser.get<ArgType>())), parser, result);
            }
            catch (std::exception& e)
            {
                std::cout << "Exception: " << e.what() << std::endl;
            }
        }

        template<typename Args, typename Self>
        static void applyMember(const Function& func, Self* self, const Args& args, TokenParser& parser, std::string& result)
        {
            using ArgType = typename FunctionTraits<Function>::template args<I>::type;
            try
            {
                Invoker<Function, I + 1, N>::applyMember(func, self, std::tuple_cat(args, std::make_tuple(parser.get<ArgType>())), parser, result);
            }
            catch (std::exception& e)
            {
                std::cout << "Exception: " << e.what() << std::endl;
            }
        }
    }; 

    template<typename Function, std::size_t N>
    class Invoker<Function, N, N>
    {
    public:
        template<typename Args>
        static void apply(const Function& func, const Args& args, TokenParser&, std::string& result)
        {
            try
            {
                // 参数列表已经准备好，可以调用function了.
                call(func, args, result);
            }
            catch (std::exception& e)
            {
                std::cout << "Exception: " << e.what() << std::endl;
            }
        }

        template<typename Args, typename Self>
        static void applyMember(const Function& func, Self* self, const Args& args, TokenParser&, std::string& result)
        {
            try
            {
                callMember(func, self, args, result);
            }  
            catch (std::exception& e)
            {
                std::cout << "Exception: " << e.what() << std::endl;
            }
        }
    };

private:
    template<typename Function>
    void bindNonMemberFunc(const std::string& protocol, const Function& func)
    {
        m_invokerMap[protocol] = { std::bind(&Invoker<Function>::template apply<std::tuple<>>, func, std::tuple<>(), 
                                             std::placeholders::_1, std::placeholders::_2), FunctionTraits<Function>::arity };
    }

    template<typename Function, typename Self>
    void bindMemberFunc(const std::string& protocol, const Function& func, Self* self)
    {
        m_invokerMap[protocol] = { std::bind(&Invoker<Function>::template applyMember<std::tuple<>, Self>, func, self, std::tuple<>(), 
                                             std::placeholders::_1, std::placeholders::_2), FunctionTraits<Function>::arity };
    }

private:
    ThreadPool m_threadPool;
    std::unordered_map<std::string, InvokerFunction> m_invokerMap;
};

}

#endif
