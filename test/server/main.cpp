#include <iostream>
#include <string>
#include <thread>
#include "gtest/gtest.h"
#include "easyrpc/EasyRpc.hpp"
#include "ProtocolDefine.hpp"

std::vector<PersonInfoRes> queryPersonInfo(const PersonInfoReq& req)
{
    EXPECT_EQ(12345678, req.cardId);
    EXPECT_STREQ("Jack", req.name.c_str());
    std::vector<PersonInfoRes> vec;
    for (int i = 0; i < 2; ++i)
    {
        PersonInfoRes res;
        res.error = 0;
        res.cardId = req.cardId;
        res.name = req.name;
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
        EXPECT_STREQ("Hello world", str.c_str());
        return str;
    }
};

TEST(EasyRpcTest, ServerCase)
{
    MessageHandle hander;
    easyrpc::Server app;
    try
    {
        app.bind("sayHello", []{ std::cout << "Hello" << std::endl; });
        bool ok = app.isBind("sayHello");
        ASSERT_TRUE(ok);

        app.bind("echo", &MessageHandle::echo, &hander);
        ok = app.isBind("echo");
        ASSERT_TRUE(ok);

        app.bind("queryPersonInfo", &queryPersonInfo);
        ok = app.isBind("queryPersonInfo");
        ASSERT_TRUE(ok);

        app.listen(50051).multithreaded(10).run();
    }
    catch (std::exception& e)
    {
        logWarn(e.what());
        FAIL();
    }

    std::cin.get();
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv); 
    return RUN_ALL_TESTS();
}
