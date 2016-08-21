#include <iostream>
#include <thread>
#include "easyrpc/EasyRpc.hpp"

EASYRPC_RPC_PROTOCOL_DEFINE(add, int(int, int));
EASYRPC_RPC_PROTOCOL_DEFINE(print, void(int));

int main()
{
    easyrpc::Client client("127.0.0.1", 8888);
    client.run();

    for (int i = 0; i < 1; ++i)
    {
        try
        {
            /* auto ret = client.call(add, 1, 2); */
            /* std::cout << "ret: " << ret << std::endl; */
            int a = 100;
            client.call(print, a);
            /* client.call(print, 1); */
        }
        catch (std::exception& e)
        {
            std::cout << "Exception: " << e.what() << std::endl;
        }
    }

    std::cin.get();

    return 0;
}

