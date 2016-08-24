#include <iostream>
#include <thread>
#include "easyrpc/EasyRpc.hpp"
#include "ProtocolDefine.hpp"

EASYRPC_RPC_PROTOCOL_DEFINE(add, int(int, int));
EASYRPC_RPC_PROTOCOL_DEFINE(print, void(const std::string&, int));
EASYRPC_RPC_PROTOCOL_DEFINE(queryPersonInfo, std::vector<PersonInfoRes>(const PersonInfoReq&));

int main()
{
    easyrpc::Client client("127.0.0.1", 8888);
    /* easyrpc::Client client("127.0.0.1", 8888, 3000); */
    client.run();

    for (int i = 0; i < 1; ++i)
    {
        try
        {
            /* client.call(print, "Hello world", i); */

            auto ret = client.call(add, 1, 2);
            std::cout << "ret: " << ret << std::endl;
            
#if 0
            PersonInfoReq req { 12345678, "Jack" };
            auto vec = client.call(queryPersonInfo, req);
            for (auto& res : vec)
            {
                std::cout << "error: " << res.error << std::endl;
                std::cout << "cardId: " << res.cardId << std::endl;
                std::cout << "name: " << res.name << std::endl;
                std::cout << "age: " << res.age << std::endl;
                std::cout << "national: " << res.national << std::endl;
            }
#endif
        }
        catch (std::exception& e)
        {
            std::cout << "Rpc exception: " << e.what() << std::endl;
        }
    }

    std::cin.get();

    return 0;
}

