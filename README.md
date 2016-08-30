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
    
    服务器调用bind函数绑定handler，支持成员函数、非成员函数以及lambda表达式的绑定，设置3000ms读socket超时，启用10个Worker线程处理业务，内部IO线程使用`an io_service-per-CPU`（一个ioservice对应一个线程）模式，最大限度提升IO性能。
    
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
    
正如你所看到的，客户端像调用本地函数一样就能够完成与服务端的通信，一切都那么简洁方便，easyrpc目前只支持短连接调用，短连接的好处就是不用担心各个server的启动顺序、调用方便以及不用维护心跳，由于每次call都会去connect，所以没有长连接高效，后期可能会考虑增加长连接call，easyrpc使用boost.asio来作为网络底层，效率自然高效，boost.serialization作为序列化框架，可以用类对象、STL（vector、map等）作为函数参数。

## 开发平台

* Ubuntu16.04 LTS gcc5.3.1
* MSVC2015

## 依赖性

* [easypack][3]
* [spdlog][4]
* boost
* c++14

## Warning

* 客户端和服务端使用的boost.serialization务必统一，要么使用32位的，要么使用64位，不然不能够通信，因为boost.serialization使用std::size_t来存储字节长度，std::size_t在32位下为unsigned int，在64位下为unsigned long。

## TODO

* 增加长连接调用。
* 增加发布/订阅模式。
* 可能会增加其他序列化框架和协议（json、msgpack等）。
* 服务注册、发现。


## License
This software is licensed under the [MIT license][5]. © 2016 chxuan


  [1]: https://github.com/topcpporg/rest_rpc
  [2]: http://img.shields.io/badge/license-MIT-blue.svg?style=flat-square
  [3]: https://github.com/chxuan/easypack
  [4]: https://github.com/gabime/spdlog
  [5]: https://github.com/chxuan/smartdb/blob/master/LICENSE
