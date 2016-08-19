#ifndef _PROTOCOL_H
#define _PROTOCOL_H

#include <string>
#include <type_traits>
#include "base/FunctionTraits.hpp"
#include "easypack/EasyPack.hpp"

#define EASYRPC_RPC_PROTOCOL_DEFINE(funcName, funcType) const static easyrpc::ProtocolDefine<funcType> handler{ #funcName }

namespace easyrpc
{

template<typename Function>
class ProtocolDefine;

template<typename Return, typename... Args>
class ProtocolDefine<Return(Args...)>
{
public:
    using ReturnType = typename FunctionTraits<Return(Args...)>::returnType;
    explicit ProtocolDefine(std::string funcName) : m_funcName(std::move(funcName)) {}

    std::string pack(Args&&... args)
    {
        easypack::Pack p;
        p.pack(std::forward<Args>(args)...);
        return p.getString();
    }

    ReturnType unpack(const std::string& text)
    {
        easypack::UnPack up(text);
        ReturnType ret;
        up.unpack(ret);
        return ret;
    }

    const std::string& funcName()
    {
        return m_funcName;
    }
    
private:
    std::string m_funcName;
};

}

#endif
