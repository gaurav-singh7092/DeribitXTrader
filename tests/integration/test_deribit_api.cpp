#include <gtest/gtest.h>
#include "exchange_interface/market_api.h"
#include "network/socket_client.h"
#include "security/credentials.h"
#include <string>
#include <thread>
#include <chrono>


class DeribitApiIntegrationTest : public ::testing::Test {
protected:
    SocketEndpoint endpoint;
    int connectionId = -1;

    void SetUp() override {
        
        channelSubscriptions.clear();
        AUTHENTICATION_SENT = false;

        
        connectionId = endpoint.connect("wss://test.deribit.com/ws/api/v2");

        
        std::this_thread::sleep_for(std::chrono::seconds(1));

        
        ASSERT_NE(connectionId, -1);
        ASSERT_TRUE(endpoint.get_metadata(connectionId));

        
        
    }

    void TearDown() override {
        
        if (connectionId != -1) {
            endpoint.close(connectionId);
        }

        
        channelSubscriptions.clear();
        AUTHENTICATION_SENT = false;
    }
};


TEST_F(DeribitApiIntegrationTest, PublicEndpoints) {
    
    jsonrpc_request timeRequest;
    timeRequest["method"] = "public/get_time";

    
    int result = endpoint.send(connectionId, timeRequest.dump());
    EXPECT_EQ(result, 0);

    
    std::this_thread::sleep_for(std::chrono::seconds(1));

    
    auto metadata = endpoint.get_metadata(connectionId);
    ASSERT_TRUE(metadata);
    EXPECT_GT(metadata->m_received_data.size(), 0);

    
    std::string lastMessage = metadata->m_received_data.back();
    EXPECT_NE(lastMessage.find("RECEIVED"), std::string::npos);
    EXPECT_NE(lastMessage.find("result"), std::string::npos);
}


TEST_F(DeribitApiIntegrationTest, OrderbookRetrieval) {
    
    jsonrpc_request orderbookRequest;
    orderbookRequest["method"] = "public/get_order_book";
    orderbookRequest["params"] = {
        {"instrument_name", "BTC-PERPETUAL"},
        {"depth", 5}
    };

    
    int result = endpoint.send(connectionId, orderbookRequest.dump());
    EXPECT_EQ(result, 0);

    
    std::this_thread::sleep_for(std::chrono::seconds(1));

    
    auto metadata = endpoint.get_metadata(connectionId);
    ASSERT_TRUE(metadata);
    EXPECT_GT(metadata->m_received_data.size(), 0);

    
    std::string lastMessage = metadata->m_received_data.back();
    EXPECT_NE(lastMessage.find("RECEIVED"), std::string::npos);
    EXPECT_NE(lastMessage.find("result"), std::string::npos);
    EXPECT_NE(lastMessage.find("asks"), std::string::npos);
    EXPECT_NE(lastMessage.find("bids"), std::string::npos);
}


TEST_F(DeribitApiIntegrationTest, SubscriptionHandling) {
    
    api::registerSubscription("BTC");

    
    auto subscriptions = api::getActiveSubscription();
    EXPECT_EQ(subscriptions.size(), 1);
    EXPECT_EQ(subscriptions[0], "deribit_price_index.BTC");

    
    jsonrpc_request subscribeRequest;
    subscribeRequest["method"] = "public/subscribe";
    subscribeRequest["params"] = {
        {"channels", subscriptions}
    };

    
    int result = endpoint.send(connectionId, subscribeRequest.dump());
    EXPECT_EQ(result, 0);

    
    std::this_thread::sleep_for(std::chrono::seconds(1));

    
    auto metadata = endpoint.get_metadata(connectionId);
    ASSERT_TRUE(metadata);
    EXPECT_GT(metadata->m_received_data.size(), 0);

    
    jsonrpc_request unsubscribeRequest;
    unsubscribeRequest["method"] = "public/unsubscribe";
    unsubscribeRequest["params"] = {
        {"channels", subscriptions}
    };

    
    result = endpoint.send(connectionId, unsubscribeRequest.dump());
    EXPECT_EQ(result, 0);

    
    std::this_thread::sleep_for(std::chrono::seconds(1));

    
    bool removeResult = api::removeActiveSubscription("BTC");
    EXPECT_TRUE(removeResult);

    
    subscriptions = api::getActiveSubscription();
    EXPECT_EQ(subscriptions.size(), 0);
}
