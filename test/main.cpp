#include <iostream>
#include <thread>
#include "easyrpc/EasyRpc.hpp"

int add(int a, int b)
{
    std::cout << "add thread id: " << std::this_thread::get_id() << std::endl;
    std::cout << "add: " << a + b << std::endl;
    return a + b;
}

int main()
{
    easyrpc::Server server;
    server.bind("add", &add);

    easypack::Pack p;
    p.pack(1, 2);
    server.route("add", p.getString());

    std::cin.get();

    return 0;
}

