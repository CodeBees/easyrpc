A RPC framework written in Modern C++
===============================================

> easyrpc是一个纯c++14开发，header-only，使用方便的RPC库，灵感来自于[topcpporg/rest_rpc][1]。

![License][2] 

* **服务端代码**

    ```cpp
    // server.cpp
    #include "easyrpc/EasyRpc.hpp"
    
    std::string echo(const std::string& str)
    {
        return str;
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
        easyrpc::Server app;
        Utils u;
        app.bind("sayHello", []
    	{ 
    		std::cout << "Hello" << std::endl;
    	});
    	app.bind("echo", &echo);
    	app.bind("add", &Utils::add, &u);
    	
        app.listen(50051).timeout(3000).multithreaded(10).run();
        
        std::cin.get();
        return 0;
    }
    ```
    
* **客户端代码**
    ```cpp
    // client.cpp
    #include "easyrpc/EasyRpc.hpp"
    
    EASYRPC_RPC_PROTOCOL_DEFINE(sayHello, void());
    EASYRPC_RPC_PROTOCOL_DEFINE(echo, std::string(const std::string&));
    EASYRPC_RPC_PROTOCOL_DEFINE(add, int(int, int));
    
    int main()
    {
        easyrpc::Client app;
        app.connect("localhost:50051").timeout(5000).run();
    	app.call(sayHello);
    	auto str = app.call(echo, "Hello world");
    	auto ret = app.call(add, 1, 2);
    
        return 0;
    }
    ```
    
以上是最简洁的客户端、服务器通信的代码，正如你所看到的，客户端像调用本地函数一样就能够完成与服务端的通信，一切都那么简洁方便，服务端bind的函数支持成员函数、非成员函数以及lambda表达式，easyrpc使用boost.asio来作为网络底层，效率自然高效，boost.serialization作为序列化框架，可以用类对象、STL作为函数参数。

## 依赖性

* [easypack][3]
* [spdlog][4]
* boost
* c++14


## License
This software is licensed under the [MIT license][5]. © 2016 chxuan


  [1]: https://github.com/topcpporg/rest_rpc
  [2]: http://img.shields.io/badge/license-MIT-blue.svg?style=flat-square
  [3]: https://github.com/chxuan/easypack
  [4]: https://github.com/gabime/spdlog
  [5]: https://github.com/chxuan/smartdb/blob/master/LICENSE
