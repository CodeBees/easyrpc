#ifndef _TOKENPARSER_H
#define _TOKENPARSER_H

#include <type_traits>
#include "easypack/EasyPack.hpp"

namespace easyrpc
{

template<typename... Args>
std::string pack(Args... args)
{
    easypack::Pack p;
    p.pack(std::forward<Args>(args)...);
    return p.getString();
}

class TokenParser
{
public:
    TokenParser() = default;
    TokenParser(const TokenParser&) = delete;
    TokenParser& operator=(const TokenParser&) = delete;

    TokenParser(const std::string& text) : m_up(text) {}

    template<typename T>
    typename std::decay<T>::type get()
    {
        using ReturnType = typename std::decay<T>::type;
        ReturnType t;
        m_up.unpackTop(t);
        return t;
    }

private:
    easypack::UnPack m_up;
};

}
#endif
