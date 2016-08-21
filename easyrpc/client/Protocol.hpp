#ifndef _PROTOCOL_H
#define _PROTOCOL_H

#include <string>
#include <type_traits>
#include "base/FunctionTraits.hpp"
#include "easypack/EasyPack.hpp"

#define EASYRPC_RPC_PROTOCOL_DEFINE(handler, funcType) const static easyrpc::ProtocolDefine<funcType> handler { #handler }

namespace easyrpc
{

template<typename Function>
class ProtocolDefine;

template<typename Return, typename... Args>
class ProtocolDefine<Return(Args...)>
{
public:
    using ReturnType = typename FunctionTraits<Return(Args...)>::returnType;
    explicit ProtocolDefine(std::string name) : m_name(std::move(name)) {}

    std::string pack(Args... args) const
    {
        easypack::Pack p;
        p.pack(std::forward<Args>(args)...);
        return p.getString();
    }

    ReturnType unpack(const std::string& text) const
    {
        easypack::UnPack up(text);
        ReturnType ret;
        up.unpack(ret);
        return ret;
    }

    const std::string& name() const
    {
        return m_name;
    }
    
private:
    std::string m_name;
};

}

#endif
