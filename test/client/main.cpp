#include <iostream>
#include <thread>
#include "easyrpc/EasyRpc.hpp"

EASYRPC_RPC_PROTOCOL_DEFINE(add, int(int, int));

int main()
{
    for (int i = 0; i < 1000; ++i)
    {

    easyrpc::Client client;
    try
    {
        client.run();
        /* client.connect("127.0.0.1", 8888); */
        client.connect("192.168.244.133", 1234);
        auto ret = client.call(add, 1, 2);
        std::cout << "ret: " << ret << std::endl;
    }
    catch (std::exception& e)
    {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    }

    std::cin.get();

    return 0;
}

