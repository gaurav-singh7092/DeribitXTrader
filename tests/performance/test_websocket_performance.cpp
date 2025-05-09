#include <gtest/gtest.h>
#include <chrono>
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include "network/socket_client.h"
#include "exchange_interface/market_api.h"

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


class WebSocketPerformanceTest : public ::testing::Test {
protected:
    SocketEndpoint endpoint;
    int connectionId = -1;
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<bool> message_received{false};
    std::atomic<int> message_count{0};

    
    const int connection_iterations = 5;
    const int message_iterations = 100;

    void SetUp() override {
        
    }

    void TearDown() override {
        
        if (connectionId != -1) {
            endpoint.close(connectionId);
            connectionId = -1;
        }
    }

    
    bool connectToEchoServer() {
        connectionId = endpoint.connect("wss://echo.websocket.org");

        
        std::this_thread::sleep_for(std::chrono::seconds(1));

        
        if (connectionId == -1 || !endpoint.get_metadata(connectionId)) {
            return false;
        }

        return true;
    }
};


TEST_F(WebSocketPerformanceTest, ConnectionPerformance) {
    PerformanceTimer timer("WebSocket Connection", connection_iterations);

    for (int i = 0; i < connection_iterations; i++) {
        
        int connId = endpoint.connect("wss://echo.websocket.org");

        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        
        ASSERT_NE(connId, -1);
        ASSERT_TRUE(endpoint.get_metadata(connId));

        
        endpoint.close(connId);

        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}


TEST_F(WebSocketPerformanceTest, MessageSendingPerformance) {
    
    ASSERT_TRUE(connectToEchoServer());

    
    std::string small_message = "Hello, WebSocket!";
    std::string medium_message(1024, 'A'); 
    std::string large_message(10240, 'B'); 

    
    {
        PerformanceTimer timer("Small Message Sending", message_iterations);
        for (int i = 0; i < message_iterations; i++) {
            int result = endpoint.send(connectionId, small_message);
            ASSERT_EQ(result, 0);

            
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    
    {
        PerformanceTimer timer("Medium Message Sending", message_iterations / 2);
        for (int i = 0; i < message_iterations / 2; i++) {
            int result = endpoint.send(connectionId, medium_message);
            ASSERT_EQ(result, 0);

            
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    }

    
    {
        PerformanceTimer timer("Large Message Sending", message_iterations / 10);
        for (int i = 0; i < message_iterations / 10; i++) {
            int result = endpoint.send(connectionId, large_message);
            ASSERT_EQ(result, 0);

            
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
}


TEST_F(WebSocketPerformanceTest, DISABLED_DeribitConnectionPerformance) {
    PerformanceTimer timer("Deribit Connection", connection_iterations);

    for (int i = 0; i < connection_iterations; i++) {
        
        int connId = endpoint.connect("wss://test.deribit.com/ws/api/v2");

        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        
        ASSERT_NE(connId, -1);
        ASSERT_TRUE(endpoint.get_metadata(connId));

        
        jsonrpc_request pingRequest;
        pingRequest["method"] = "public/ping";

        
        int result = endpoint.send(connId, pingRequest.dump());
        ASSERT_EQ(result, 0);

        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        
        endpoint.close(connId);

        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}


TEST_F(WebSocketPerformanceTest, DISABLED_MultipleConnectionsPerformance) {
    const int num_connections = 5;
    std::vector<int> connectionIds;

    PerformanceTimer timer("Multiple Connections", num_connections);

    
    for (int i = 0; i < num_connections; i++) {
        int connId = endpoint.connect("wss://echo.websocket.org");
        connectionIds.push_back(connId);

        
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        
        ASSERT_NE(connId, -1);
        ASSERT_TRUE(endpoint.get_metadata(connId));
    }

    
    for (int connId : connectionIds) {
        int result = endpoint.send(connId, "Hello from connection " + std::to_string(connId));
        ASSERT_EQ(result, 0);
    }

    
    std::this_thread::sleep_for(std::chrono::seconds(1));

    
    for (int connId : connectionIds) {
        endpoint.close(connId);
    }

    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}


