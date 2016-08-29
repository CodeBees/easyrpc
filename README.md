A RPC framework written in Modern C++
===============================================

> easyrpc是一个纯c++14开发，header-only，使用方便的RPC库，灵感来自于[topcpporg/rest_rpc][1]。

![License][2] 

* **一个最简洁的服务端.**

    ```cpp
    #include "easyrpc/EasyRpc.hpp"
    
    int main()
    {
        easyrpc::Server app;
        app.bind("sayHello", []
    	{ 
    		std::cout << "Hello" << std::endl;
    	});
    	
        app.listen(50051).run();
        
        std::cin.get();
        return 0;
    }
    ```
    
* **一个最简洁的客户端.**
    ```cpp
    #include "easyrpc/EasyRpc.hpp"
    
    EASYRPC_RPC_PROTOCOL_DEFINE(sayHello, void());
    
    int main()
    {
        easyrpc::Client app;
        app.connect("localhost:50051").run();
    	app.call(sayHello);
    
        return 0;
    }
    ```

## 依赖性

* boost
* c++14

## 兼容性

* `Linux x86_64` gcc 4.8, gcc4.9, gcc 5.
* `Windows x86_64` Visual Studio 2015

## License
This software is licensed under the [MIT license][3]. © 2016 chxuan


  [1]: https://github.com/topcpporg/rest_rpc
  [2]: http://img.shields.io/badge/license-MIT-blue.svg?style=flat-square
  [3]: https://github.com/chxuan/smartdb/blob/master/LICENSE
