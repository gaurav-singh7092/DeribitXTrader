#include <gtest/gtest.h>
#include "network/socket_client.h"
#include <string>
#include <memory>


class MockConnectionDetails : public ConnectionDetails {
public:
    MockConnectionDetails(int id, const std::string& uri)
        : ConnectionDetails(id, uri, nullptr) {
        
    }

    
    void setup_websocket() {
        
    }

    
    using ConnectionDetails::get_id;
    using ConnectionDetails::get_status;
    using ConnectionDetails::get_uri;
    using ConnectionDetails::get_server;
    using ConnectionDetails::get_error_reason;
    using ConnectionDetails::record_sent_message;
};


class SocketClientTest : public ::testing::Test {
protected:
    void SetUp() override {
        
    }

    void TearDown() override {
        
    }
};


TEST_F(SocketClientTest, ConnectionDetailsBasics) {
    const int test_id = 42;
    const std::string test_uri = "wss://test.example.com/ws";

    MockConnectionDetails connection(test_id, test_uri);

    
    EXPECT_EQ(connection.get_id(), test_id);
    EXPECT_EQ(connection.get_uri(), test_uri);
    EXPECT_EQ(connection.get_status(), "Connecting");
    EXPECT_EQ(connection.get_server(), "N/A");
    EXPECT_TRUE(connection.get_error_reason().empty());

    
    EXPECT_TRUE(connection.m_received_data.empty());
    connection.record_sent_message("test message");
    EXPECT_EQ(connection.m_received_data.size(), 1);
    EXPECT_EQ(connection.m_received_data[0], "SENT: test message");
}


TEST_F(SocketClientTest, SocketEndpointBasics) {
    SocketEndpoint endpoint;

    
    auto metadata = endpoint.get_metadata(999);
    EXPECT_FALSE(metadata); 

    
    endpoint.close(999);

    
    int result = endpoint.send(999, "test message");
    EXPECT_EQ(result, -1); 
}


TEST_F(SocketClientTest, ConnectionDetailsSharedPtr) {
    
    ConnectionDetails::ptr ptr(new MockConnectionDetails(1, "wss://test.example.com/ws"));

    
    EXPECT_TRUE(ptr);

    
    EXPECT_EQ(ptr->get_id(), 1);
    EXPECT_EQ(ptr->get_uri(), "wss://test.example.com/ws");

    
    ConnectionDetails::ptr ptr2 = ptr;
    EXPECT_EQ(ptr.use_count(), 2);

    
    ptr.reset();
    EXPECT_FALSE(ptr);
    EXPECT_TRUE(ptr2);
    EXPECT_EQ(ptr2.use_count(), 1);
}
