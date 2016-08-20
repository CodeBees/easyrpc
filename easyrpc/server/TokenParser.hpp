#ifndef _TOKENPARSER_H
#define _TOKENPARSER_H

#include "easypack/EasyPack.hpp"

namespace easyrpc
{

template<typename T>
std::string pack(T& t)
{
    easypack::Pack p;
    p.pack(t);
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
    T get()
    {
        T t;
        m_up.unpackTop(t);
        return t;
    }

private:
    easypack::UnPack m_up;
};

}
#endif
