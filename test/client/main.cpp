#include <iostream>
#include <thread>
#include "easyrpc/EasyRpc.hpp"
#include "ProtocolDefine.hpp"

EASYRPC_RPC_PROTOCOL_DEFINE(add, int(int, int));
EASYRPC_RPC_PROTOCOL_DEFINE(print, bool(const std::string&, int));
EASYRPC_RPC_PROTOCOL_DEFINE(toUpper, std::string(const std::string&));
EASYRPC_RPC_PROTOCOL_DEFINE(queryPersonInfo, PersonInfoRes(const PersonInfoReq&));

int main()
{
    easyrpc::Client client("127.0.0.1", 8888);
    client.run();

    for (int i = 0; i < 100; ++i)
    {
        try
        {
            auto ret = client.call(add, 1, 2);
            std::cout << "ret: " << ret << std::endl;
            
            auto ok = client.call(print, "Hello world", i);
            std::cout << ok << std::endl;

            auto str = client.call(toUpper, "nihao");
            std::cout << str << std::endl;

            PersonInfoReq req { 12345678, "Jack" };
            auto res = client.call(queryPersonInfo, req);
            std::cout << "cardId: " << res.cardId << std::endl;
            std::cout << "name: " << res.name << std::endl;
            std::cout << "age: " << res.age << std::endl;
            std::cout << "national: " << res.national << std::endl;
        }
        catch (std::exception& e)
        {
            std::cout << "Exception: " << e.what() << std::endl;
        }
    }

    std::cin.get();

    return 0;
}

