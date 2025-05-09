#include <gtest/gtest.h>
#include <chrono>
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include "exchange_interface/market_api.h"
#include "network/socket_client.h"

using namespace std::chrono;


class PerformanceTimer {
private:
    high_resolution_clock::time_point start_time;
    std::string operation_name;
    int iterations;

public:
    PerformanceTimer(const std::string& name, int iters)
        : operation_name(name), iterations(iters) {
        start_time = high_resolution_clock::now();
    }

    ~PerformanceTimer() {
        auto end_time = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(end_time - start_time).count();
        double avg_duration = static_cast<double>(duration) / iterations;

        std::cout << std::fixed << std::setprecision(3);
        std::cout << "PERFORMANCE [" << operation_name << "]: "
                  << duration << " µs total, "
                  << avg_duration << " µs per operation ("
                  << iterations << " iterations)" << std::endl;
    }
};


class MarketApiPerformanceTest : public ::testing::Test {
protected:
    
    const int request_iterations = 1000;
    const int validation_iterations = 10000;

    void SetUp() override {
        
        channelSubscriptions.clear();
    }

    void TearDown() override {
        
        channelSubscriptions.clear();
    }
};


TEST_F(MarketApiPerformanceTest, InstrumentNameValidationPerformance) {
    
    std::vector<std::string> valid_instruments = {
        "BTC-PERPETUAL", "ETH-PERPETUAL", "SOL-PERPETUAL",
        "BTC-31DEC24", "ETH-30JUN23", "SOL-31MAR24"
    };

    
    std::vector<std::string> invalid_instruments = {
        "BTC", "ETH", "SOL",
        "BTC-", "ETH-", "SOL-",
        "BTC-PERP", "ETH-PERP", "SOL-PERP",
        "BTC-31DEC2024", "ETH-30JUN2023", "SOL-31MAR2024",
        "btc-perpetual", "eth-perpetual", "sol-perpetual"
    };

    
    {
        PerformanceTimer timer("Valid Instrument Validation", validation_iterations);
        for (int i = 0; i < validation_iterations; i++) {
            const std::string& instrument = valid_instruments[i % valid_instruments.size()];
            bool result = api::is_valid_instrument_name(instrument);
            ASSERT_TRUE(result);
        }
    }

    
    {
        PerformanceTimer timer("Invalid Instrument Validation", validation_iterations);
        for (int i = 0; i < validation_iterations; i++) {
            const std::string& instrument = invalid_instruments[i % invalid_instruments.size()];
            bool result = api::is_valid_instrument_name(instrument);
            ASSERT_FALSE(result);
        }
    }
}


TEST_F(MarketApiPerformanceTest, SubscriptionManagementPerformance) {
    
    {
        PerformanceTimer timer("Add Subscriptions", request_iterations);
        for (int i = 0; i < request_iterations; i++) {
            std::string index_name = "BTC" + std::to_string(i % 10);
            api::registerSubscription(index_name);
        }
    }

    
    {
        PerformanceTimer timer("Get Subscriptions", request_iterations);
        for (int i = 0; i < request_iterations; i++) {
            std::vector<std::string> subscriptions = api::getActiveSubscription();
            ASSERT_FALSE(subscriptions.empty());
        }
    }

    
    {
        PerformanceTimer timer("Remove Subscriptions", request_iterations);
        for (int i = 0; i < request_iterations; i++) {
            std::string index_name = "BTC" + std::to_string(i % 10);
            api::removeActiveSubscription(index_name);
        }
    }
}


TEST_F(MarketApiPerformanceTest, JsonrpcRequestCreationPerformance) {
    
    {
        PerformanceTimer timer("JSONRPC Default Constructor", request_iterations);
        for (int i = 0; i < request_iterations; i++) {
            jsonrpc_request req;
            ASSERT_EQ(req["jsonrpc"], "2.0");
            ASSERT_TRUE(req.contains("id"));
        }
    }

    
    {
        PerformanceTimer timer("JSONRPC Method Constructor", request_iterations);
        for (int i = 0; i < request_iterations; i++) {
            std::string method = "test_method_" + std::to_string(i % 10);
            jsonrpc_request req(method);
            ASSERT_EQ(req["jsonrpc"], "2.0");
            ASSERT_EQ(req["method"], method);
            ASSERT_TRUE(req.contains("id"));
        }
    }
}


TEST_F(MarketApiPerformanceTest, RequestProcessingPerformance) {
    
    std::vector<std::string> test_requests = {
        "deribit 1 orderbook BTC-PERPETUAL 5",
        "deribit 1 subscribe BTC",
        "deribit 1 unsubscribe BTC",
        "deribit 1 positions",
        "deribit 1 get_open_orders"
    };

    
    {
        PerformanceTimer timer("API Request Processing", request_iterations);
        for (int i = 0; i < request_iterations; i++) {
            const std::string& request = test_requests[i % test_requests.size()];
            std::string result = api::processRequest(request);
            
            
        }
    }
}


