#include <iostream>
#include <thread>
#include "easyrpc/EasyRpc.hpp"

EASYRPC_RPC_PROTOCOL_DEFINE(add, int(int, int));

int main()
{
    easyrpc::Client client;
    try
    {
        client.run();
        client.connect("127.0.0.1", 8888);
    }
    catch (std::exception& e)
    {
        std::cout << "Exception: " << e.what() << std::endl;
    }

    return 0;
}

