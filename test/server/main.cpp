#include <iostream>
#include <string>
#include <thread>
#include "easyrpc/EasyRpc.hpp"
#include "ProtocolDefine.hpp"

void print(const std::string& str, int i)
{
    std::cout << str << ", " << i << std::endl;
}

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

class Utils
{
public:
    int add(int a, int b)
    {
        return a + b;
    }
};

int main()
{
    Utils u;

    easyrpc::Server app;
    try
    {
        app.bind("hi", []{ std::cout << "Say Hi!" << std::endl; });
        app.bind("print", &print);
        app.bind("add", &Utils::add, &u);
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

