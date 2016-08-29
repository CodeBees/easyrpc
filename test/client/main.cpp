#include <iostream>
#include <thread>
#include "gtest/gtest.h"
#include "easyrpc/EasyRpc.hpp"
#include "ProtocolDefine.hpp"

EASYRPC_RPC_PROTOCOL_DEFINE(sayHello, void());
EASYRPC_RPC_PROTOCOL_DEFINE(echo, std::string(const std::string&));
EASYRPC_RPC_PROTOCOL_DEFINE(queryPersonInfo, std::vector<PersonInfoRes>(const PersonInfoReq&));

TEST(EasyRpcTest, ClientCase)
{
    easyrpc::Client app;

    try
    {
        app.connect("localhost:50051").run();

        app.call(sayHello);
        std::string ret = app.call(echo, "Hello world");
        EXPECT_STREQ("Hello world", ret.c_str());

        PersonInfoReq req { 12345678, "Jack" };
        auto vec = app.call(queryPersonInfo, req);
        EXPECT_EQ(2, static_cast<int>(vec.size()));
        for (auto& res : vec)
        {
            EXPECT_EQ(0, res.error);
            EXPECT_EQ(req.cardId, res.cardId);
            EXPECT_STREQ(req.name.c_str(), res.name.c_str());
            EXPECT_EQ(20, res.age);
            EXPECT_STREQ("han", res.national.c_str());
        }
    }
    catch (std::exception& e)
    {
        logWarn(e.what());
        FAIL();
    }
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv); 
    return RUN_ALL_TESTS();
}

