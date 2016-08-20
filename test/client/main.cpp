#include <iostream>
#include <thread>
#include "easyrpc/EasyRpc.hpp"
#include "easyrpc/client/Protocol.hpp"
using namespace easyrpc;

EASYRPC_RPC_PROTOCOL_DEFINE(add, int(int, int));

int main()
{
    easyrpc::Client client;
    try
    {
        client.run();
        client.connect("127.0.0.1", 8888);
        auto ret = client.call(add, 1, 2);
        std::cout << "ret: " << ret << std::endl;
    }
    catch (std::exception& e)
    {
        std::cout << "Exception: " << e.what() << std::endl;
    }

    return 0;
}

