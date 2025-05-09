#include <gtest/gtest.h>
#include "network/socket_client.h"
#include "exchange_interface/market_api.h"
#include <string>
#include <thread>
#include <chrono>


class WebSocketConnectionTest : public ::testing::Test {
protected:
    SocketEndpoint endpoint;

    void SetUp() override {
        
    }

    void TearDown() override {
        
    }
};


TEST_F(WebSocketConnectionTest, EchoServerConnection) {
    
    int connectionId = endpoint.connect("wss://echo.websocket.org");

    
    std::this_thread::sleep_for(std::chrono::seconds(1));

    
    ASSERT_NE(connectionId, -1);
    ASSERT_TRUE(endpoint.get_metadata(connectionId));
    ASSERT_EQ(endpoint.get_metadata(connectionId)->get_status(), "Connected");

    
    const std::string testMessage = "Hello, WebSocket!";
    int result = endpoint.send(connectionId, testMessage);
    EXPECT_EQ(result, 0);

    
    std::this_thread::sleep_for(std::chrono::seconds(1));

    
    auto metadata = endpoint.get_metadata(connectionId);
    ASSERT_TRUE(metadata);

    
    

    
    endpoint.close(connectionId);

    
    std::this_thread::sleep_for(std::chrono::seconds(1));

    
    
}


TEST_F(WebSocketConnectionTest, DeribitConnection) {
    
    int connectionId = endpoint.connect("wss://test.deribit.com/ws/api/v2");

    
    std::this_thread::sleep_for(std::chrono::seconds(1));

    
    ASSERT_NE(connectionId, -1);
    ASSERT_TRUE(endpoint.get_metadata(connectionId));
    ASSERT_EQ(endpoint.get_metadata(connectionId)->get_status(), "Connected");

    
    jsonrpc_request pingRequest;
    pingRequest["method"] = "public/ping";

    
    int result = endpoint.send(connectionId, pingRequest.dump());
    EXPECT_EQ(result, 0);

    
    std::this_thread::sleep_for(std::chrono::seconds(1));

    
    auto metadata = endpoint.get_metadata(connectionId);
    ASSERT_TRUE(metadata);
    EXPECT_GT(metadata->m_received_data.size(), 0);

    
    std::string lastMessage = metadata->m_received_data.back();
    EXPECT_NE(lastMessage.find("RECEIVED"), std::string::npos);
    EXPECT_NE(lastMessage.find("result"), std::string::npos);

    
    endpoint.close(connectionId);

    
    std::this_thread::sleep_for(std::chrono::seconds(1));

    
    
}


TEST_F(WebSocketConnectionTest, MultipleConnections) {
    
    int connectionId1 = endpoint.connect("wss://test.deribit.com/ws/api/v2");
    int connectionId2 = endpoint.connect("wss://echo.websocket.org");

    
    std::this_thread::sleep_for(std::chrono::seconds(1));

    
    ASSERT_NE(connectionId1, -1);
    ASSERT_NE(connectionId2, -1);
    ASSERT_TRUE(endpoint.get_metadata(connectionId1));
    ASSERT_TRUE(endpoint.get_metadata(connectionId2));
    ASSERT_EQ(endpoint.get_metadata(connectionId1)->get_status(), "Connected");
    ASSERT_EQ(endpoint.get_metadata(connectionId2)->get_status(), "Connected");

    
    EXPECT_NE(connectionId1, connectionId2);

    
    endpoint.close(connectionId1);
    endpoint.close(connectionId2);

    
    std::this_thread::sleep_for(std::chrono::seconds(1));

    
    
}


TEST_F(WebSocketConnectionTest, ConnectionErrorHandling) {
    
    int connectionId = endpoint.connect("wss://invalid.example.com");

    
    std::this_thread::sleep_for(std::chrono::seconds(2));

    
    ASSERT_NE(connectionId, -1);
    ASSERT_TRUE(endpoint.get_metadata(connectionId));
    EXPECT_EQ(endpoint.get_metadata(connectionId)->get_status(), "Error");
    EXPECT_FALSE(endpoint.get_metadata(connectionId)->get_error_reason().empty());
}
