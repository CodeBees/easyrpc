#include <iostream>
#include <string>
#include <thread>
#include "easyrpc/EasyRpc.hpp"
#include "ProtocolDefine.hpp"

std::vector<PersonInfoRes> queryPersonInfo(const PersonInfoReq& req)
{
    std::cout << "cardId: " << req.cardId << std::endl;
    std::cout << "name: " << req.name << std::endl;
    std::vector<PersonInfoRes> vec;
    for (int i = 0; i < 2; ++i)
    {
        PersonInfoRes res;
        res.error = 1;
        res.cardId = 11111;
        res.name = "Jack";
        res.age = 20;
        res.national = "han";
        vec.emplace_back(std::move(res));
    }
    return vec;
}

class MessageHandle
{
public:
    std::string echo(const std::string& str)
    {
        return str;
    }
};

int main()
{

    MessageHandle hander;
    easyrpc::Server app;
    try
    {
        app.bind("sayHello", []{ std::cout << "Hello" << std::endl; });
        app.bind("echo", &MessageHandle::echo, &hander);
        app.bind("queryPersonInfo", &queryPersonInfo);
        app.listen(8888).multithreaded(10).run();
    }
    catch (std::exception& e)
    {
        std::cout << "Rpc exception: " << e.what() << std::endl;
    }

    std::cout << "Server start..." << std::endl;
    std::cin.get();
    app.stop();
    std::cout << "Server stop..." << std::endl;

    return 0;
}

