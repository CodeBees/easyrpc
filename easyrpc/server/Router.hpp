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

    void operator()(const std::string& text)
    {
        TokenParser parser(text);
        std::string result;
        m_func(parser, result);
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

    static Router& instance()
    {
        static Router router;
        return router;
    }

    void setThreadPoolSize(std::size_t size)
    {
        m_threadPool.initThreadNum(size);
    }

    template<typename Function>
    void bind(const std::string& funcName, const Function& func)
    {
        bindNonMemberFunc(funcName, func);
    }

    template<typename Function, typename Self>
    void bind(const std::string& funcName, const Function& func, Self* self)
    {
        bindMemberFunc(funcName, func, self); 
    }

    void route(const std::string& funcName, const std::string& text)
    {
        auto iter = m_invokerMap.find(funcName);
        if (iter == m_invokerMap.end())
        {
            return;
        }

        m_threadPool.addTask(iter->second, text);
    }

private:
    template<typename Function, typename... Args>
    static typename std::enable_if<std::is_void<typename std::result_of<Function(Args...)>::type>::value>::type
    call(const Function& func, const std::tuple<Args...>& tp, std::string&)
    {
        callImpl(func, std::make_index_sequence<sizeof...(Args)>{}, tp);
    }

    template<typename Function, typename... Args>
    static typename std::enable_if<!std::is_void<typename std::result_of<Function(Args...)>::type>::value>::type
    call(const Function& func, const std::tuple<Args...>& tp, std::string& result)
    {
        auto ret = callImpl(func, std::make_index_sequence<sizeof...(Args)>{}, tp);
        // result = ret;
    }

    template<typename Function, std::size_t... I, typename... Args>
    static auto callImpl(const Function& func, const std::index_sequence<I...>&, const std::tuple<Args...>& tp)
    {
        return func(std::get<I>(tp)...);
    }

    template<typename Function, typename Self, typename... Args>
    static typename std::enable_if<std::is_void<typename std::result_of<Function(Self, Args...)>::type>::value>::type
    callMember(const Function& func, Self* self, const std::tuple<Args...>& tp, std::string&)
    {
        callMemberImpl(func, self, std::make_index_sequence<sizeof...(Args)>{}, tp);
    }

    template<typename Function, typename Self, typename... Args>
    static typename std::enable_if<!std::is_void<typename std::result_of<Function(Self, Args...)>::type>::value>::type
    callMember(const Function& func, Self* self, const std::tuple<Args...>& tp, std::string& result)
    {
        auto ret = callMemberImpl(func, self, std::make_index_sequence<sizeof...(Args)>{}, tp);
        // result = ret;
    }

    template<typename Function, typename Self, std::size_t... I, typename... Args>
    static auto callMemberImpl(const Function& func, Self* self, const std::index_sequence<I...>&, const std::tuple<Args...>& tp)
    {
        return (*self.*func)(std::get<I>(tp)...);
    }

private:
    template<typename Function, std::size_t I = 0, std::size_t N = FunctionTraits<Function>::arity>
    class Invoker
    {
    public:
        template<typename Args>
        static void apply(const Function& func, TokenParser& parser, const Args& args, std::string& result)
        {
            using ArgType = typename FunctionTraits<Function>::template args<I>::type;
            try
            {
                Invoker<Function, I + 1, N>::apply(func, parser, std::tuple_cat(args, std::make_tuple(parser.get<ArgType>())), result);
            }
            catch (std::exception& e)
            {
                std::cout << "Exception: " << e.what() << std::endl;
            }
        }

        template<typename Args, typename Self>
        static void applyMember(const Function& func, Self* self, TokenParser& parser, const Args& args, std::string& result)
        {
            using ArgType = typename FunctionTraits<Function>::template args<I>::type;
            try
            {
                Invoker<Function, I + 1, N>::applyMember(func, self, parser, std::tuple_cat(args, std::make_tuple(parser.get<ArgType>())), result);
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
        static void apply(const Function& func, TokenParser&, const Args& args, std::string& result)
        {
            call(func, args, result);
        }

        template<typename Args, typename Self>
        static void applyMember(const Function& func, Self* self, TokenParser&, const Args& args, std::string& result)
        {
            callMember(func, self, args, result);
        }
    };

private:
    template<typename Function>
    void bindNonMemberFunc(const std::string& funcName, const Function& func)
    {
#if 0
        m_invokerMap.emplace(funcName, { std::bind(&Invoker<Function>::template apply<std::tuple<>>, func, 
                                                   std::placeholders::_1, std::placeholders::_2, 
                                                   std::tuple<>()), FunctionTraits<Function>::arity }); 
#endif
        m_invokerMap[funcName] = { std::bind(&Invoker<Function>::template apply<std::tuple<>>, func, 
                                             std::placeholders::_1, std::placeholders::_2, 
                                             std::tuple<>()), FunctionTraits<Function>::arity };
    }

    template<typename Function, typename Self>
    void bindMemberFunc(const std::string& funcName, const Function& func, Self* self)
    {
#if 0
        m_invokerMap.emplace(funcName, { std::bind(&Invoker<Function>::template applyMember<std::tuple<>, Self>, func, self,
                                                   std::placeholders::_1, std::placeholders::_2,
                                                   std::tuple<>()), FunctionTraits<Function>::arity });
#endif
        m_invokerMap[funcName] = { std::bind(&Invoker<Function>::template applyMember<std::tuple<>, Self>, func, self,
                                             std::placeholders::_1, std::placeholders::_2,
                                             std::tuple<>()), FunctionTraits<Function>::arity };
    }

private:
    ThreadPool m_threadPool;
    /* std::unordered_map<std::string, InvokerFunction> m_invokerMap; */
    std::map<std::string, InvokerFunction> m_invokerMap;
};

}

#endif
