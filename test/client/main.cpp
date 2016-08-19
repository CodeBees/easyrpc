#include <iostream>
#include <thread>
#include "easyrpc/EasyRpc.hpp"

int add(int a, int b)
{
    std::cout << "add thread id: " << std::this_thread::get_id() << std::endl;
    std::cout << "add: " << a + b << std::endl;
    return a + b;
}

class Test
{
public:
    int add2(int a, int b)
    {
        std::cout << "add2 thread id: " << std::this_thread::get_id() << std::endl;
        std::cout << "add2: " << a + b << std::endl;
        return a + b;
    }

};

int main()
{
    std::cout << "main thread id: " << std::this_thread::get_id() << std::endl;
    Test t;
    easyrpc::Server server;
    server.setThreadPoolSize(2);
    server.bind("add", &add);
    server.bind("add2", &Test::add2, &t);

    easypack::Pack p;
    p.pack(1, 2);
    server.route("add", p.getString());

    easypack::Pack p2;
    p2.pack(5, 2);
    server.route("add2", p2.getString());

    std::cin.get();

    return 0;
}

