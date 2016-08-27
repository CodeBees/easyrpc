#include <iostream>
#include <thread>
#include "easyrpc/EasyRpc.hpp"
#include "ProtocolDefine.hpp"

EASYRPC_RPC_PROTOCOL_DEFINE(sayHello, void());
EASYRPC_RPC_PROTOCOL_DEFINE(echo, std::string(const std::string&));
EASYRPC_RPC_PROTOCOL_DEFINE(queryPersonInfo, std::vector<PersonInfoRes>(const PersonInfoReq&));

int main()
{
    easyrpc::Client app;

    try
    {
        app.connect("localhost:50051").run();

        app.call(sayHello);
        std::cout << app.call(echo, "Hello world") << std::endl;

#if 0
        PersonInfoReq req { 12345678, "Jack" };
        auto vec = app.call(queryPersonInfo, req);
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
        return 1;
    }

    std::cin.get();

    return 0;
}

