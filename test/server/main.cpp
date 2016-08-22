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
        res.national = "汉";
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
    std::cout << "main thread id: " << std::this_thread::get_id() << std::endl;
    Utils u;

    std::size_t num = std::thread::hardware_concurrency();
    easyrpc::Server server("127.0.0.1", 8888, num);
    try
    {
        server.setThreadPoolSize(10);
        server.bind("print", &print);
        server.bind("add", &Utils::add, &u);
        server.bind("queryPersonInfo", &queryPersonInfo);
        server.run();
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

