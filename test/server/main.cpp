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
        /* std::this_thread::sleep_for(std::chrono::milliseconds(10000)); */
        return a + b;
    }
};

int main()
{
    Utils u;

    easyrpc::Server server;
    try
    {
        server.bind("print", &print);
        server.bind("add", &Utils::add, &u);
        server.bind("queryPersonInfo", &queryPersonInfo);
        server.listen(8888).multithreaded(10).run();
    }
    catch (std::exception& e)
    {
        std::cout << "Rpc exception: " << e.what() << std::endl;
    }

    std::cout << "Server start..." << std::endl;
    std::cin.get();
    server.stop();
    std::cout << "Server stop..." << std::endl;

    return 0;
}

