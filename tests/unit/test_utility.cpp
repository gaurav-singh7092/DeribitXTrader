#include <gtest/gtest.h>
#include "helpers/utility.h"
#include <string>
#include <map>


class UtilityTest : public ::testing::Test {
protected:
    void SetUp() override {
        
    }

    void TearDown() override {
        
    }
};


TEST_F(UtilityTest, GetCurrentTimestamp) {
    long long timestamp = utils::getCurrentTimestamp();
    
    
    const long long jan_1_2023 = 1672531200000; 
    EXPECT_GT(timestamp, jan_1_2023);
    
    
    long long timestamp2 = utils::getCurrentTimestamp();
    EXPECT_GE(timestamp2, timestamp);
}


TEST_F(UtilityTest, GenerateRandomString) {
    
    for (int length : {5, 10, 20}) {
        std::string random = utils::generateRandomString(length);
        
        
        EXPECT_EQ(random.length(), length);
        
        
        std::string random2 = utils::generateRandomString(length);
        EXPECT_NE(random, random2);
    }
    
    
    std::string empty = utils::generateRandomString(0);
    EXPECT_TRUE(empty.empty());
}


TEST_F(UtilityTest, MapToString) {
    
    std::map<std::string, std::string> testMap = {
        {"key1", "value1"},
        {"key2", "value2"},
        {"key3", "value3"}
    };
    
    
    std::string result = utils::mapToString(testMap);
    
    
    EXPECT_NE(result.find("key1"), std::string::npos);
    EXPECT_NE(result.find("value1"), std::string::npos);
    EXPECT_NE(result.find("key2"), std::string::npos);
    EXPECT_NE(result.find("value2"), std::string::npos);
    EXPECT_NE(result.find("key3"), std::string::npos);
    EXPECT_NE(result.find("value3"), std::string::npos);
}


TEST_F(UtilityTest, GetTerminalWidth) {
    int width = utils::getTerminalWidth();
    
    
    EXPECT_GE(width, 40);
    EXPECT_LE(width, 500);
}
