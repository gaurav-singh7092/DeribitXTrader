#include <gtest/gtest.h>
#include "exchange_interface/market_api.h"
#include <vector>
#include <string>


class MarketApiTest : public ::testing::Test {
protected:
    void SetUp() override {
        
        channelSubscriptions.clear();
    }

    void TearDown() override {
        
        channelSubscriptions.clear();
    }
};


TEST_F(MarketApiTest, RegisterSubscription) {
    
    EXPECT_TRUE(api::getActiveSubscription().empty());
    
    
    api::registerSubscription("BTC");
    
    
    auto subscriptions = api::getActiveSubscription();
    EXPECT_EQ(subscriptions.size(), 1);
    EXPECT_EQ(subscriptions[0], "deribit_price_index.BTC");
    
    
    api::registerSubscription("BTC");
    
    
    subscriptions = api::getActiveSubscription();
    EXPECT_EQ(subscriptions.size(), 1);
    
    
    api::registerSubscription("ETH");
    
    
    subscriptions = api::getActiveSubscription();
    EXPECT_EQ(subscriptions.size(), 2);
    EXPECT_EQ(subscriptions[0], "deribit_price_index.BTC");
    EXPECT_EQ(subscriptions[1], "deribit_price_index.ETH");
}


TEST_F(MarketApiTest, RemoveSubscription) {
    
    api::registerSubscription("BTC");
    api::registerSubscription("ETH");
    
    
    auto subscriptions = api::getActiveSubscription();
    EXPECT_EQ(subscriptions.size(), 2);
    
    
    bool result = api::removeActiveSubscription("BTC");
    EXPECT_TRUE(result);
    
    
    subscriptions = api::getActiveSubscription();
    EXPECT_EQ(subscriptions.size(), 1);
    EXPECT_EQ(subscriptions[0], "deribit_price_index.ETH");
    
    
    result = api::removeActiveSubscription("XRP");
    EXPECT_FALSE(result);
    
    
    subscriptions = api::getActiveSubscription();
    EXPECT_EQ(subscriptions.size(), 1);
}


TEST_F(MarketApiTest, InstrumentNameValidation) {
    
    EXPECT_TRUE(api::is_valid_instrument_name("BTC-PERPETUAL"));
    EXPECT_TRUE(api::is_valid_instrument_name("ETH-PERPETUAL"));
    EXPECT_TRUE(api::is_valid_instrument_name("BTC-31DEC24"));
    EXPECT_TRUE(api::is_valid_instrument_name("ETH-30JUN23"));
    
    
    EXPECT_FALSE(api::is_valid_instrument_name("BTC"));
    EXPECT_FALSE(api::is_valid_instrument_name("BTC-"));
    EXPECT_FALSE(api::is_valid_instrument_name("BTC-PERP"));
    EXPECT_FALSE(api::is_valid_instrument_name("BTC-31DEC2024"));
    EXPECT_FALSE(api::is_valid_instrument_name("btc-perpetual")); 
    EXPECT_FALSE(api::is_valid_instrument_name(""));
}


TEST_F(MarketApiTest, JsonrpcRequestCreation) {
    
    jsonrpc_request req1;
    EXPECT_EQ(req1["jsonrpc"], "2.0");
    EXPECT_TRUE(req1.contains("id"));
    
    
    jsonrpc_request req2("test_method");
    EXPECT_EQ(req2["jsonrpc"], "2.0");
    EXPECT_EQ(req2["method"], "test_method");
    EXPECT_TRUE(req2.contains("id"));
}
