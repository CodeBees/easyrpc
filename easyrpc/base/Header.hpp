#ifndef _HEADER_H
#define _HEADER_H

namespace easyrpc
{

const int MaxBufferLenght = 8192;
const int RequestHeaderLenght = 8;
const int ResponseHeaderLenght = 4;

struct RequestHeader
{
    unsigned int protocolLen;
    unsigned int bodyLen;
};

struct ResponseHeader
{
    unsigned int bodyLen;
};

}

#endif
