#include <gtest/gtest.h>
#include "data_format/json_parser.hpp"
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;


class JsonParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        
    }

    void TearDown() override {
        
    }
};


TEST_F(JsonParserTest, BasicParsing) {
    
    std::string jsonStr = R"({"name":"John","age":30,"city":"New York"})";
    
    
    json parsed = json::parse(jsonStr);
    
    
    EXPECT_EQ(parsed["name"], "John");
    EXPECT_EQ(parsed["age"], 30);
    EXPECT_EQ(parsed["city"], "New York");
}


TEST_F(JsonParserTest, ArrayParsing) {
    
    std::string jsonStr = R"({"numbers":[1,2,3,4,5],"names":["Alice","Bob","Charlie"]})";
    
    
    json parsed = json::parse(jsonStr);
    
    
    EXPECT_EQ(parsed["numbers"].size(), 5);
    EXPECT_EQ(parsed["numbers"][0], 1);
    EXPECT_EQ(parsed["numbers"][4], 5);
    
    EXPECT_EQ(parsed["names"].size(), 3);
    EXPECT_EQ(parsed["names"][0], "Alice");
    EXPECT_EQ(parsed["names"][2], "Charlie");
}


TEST_F(JsonParserTest, ObjectCreation) {
    
    json obj;
    obj["name"] = "John";
    obj["age"] = 30;
    obj["city"] = "New York";
    
    
    EXPECT_EQ(obj["name"], "John");
    EXPECT_EQ(obj["age"], 30);
    EXPECT_EQ(obj["city"], "New York");
    
    
    std::string jsonStr = obj.dump();
    EXPECT_NE(jsonStr.find("John"), std::string::npos);
    EXPECT_NE(jsonStr.find("30"), std::string::npos);
    EXPECT_NE(jsonStr.find("New York"), std::string::npos);
}


TEST_F(JsonParserTest, NestedObjects) {
    
    std::string jsonStr = R"({
        "person": {
            "name": "John",
            "age": 30,
            "address": {
                "street": "123 Main St",
                "city": "New York",
                "zip": "10001"
            }
        }
    })";
    
    
    json parsed = json::parse(jsonStr);
    
    
    EXPECT_EQ(parsed["person"]["name"], "John");
    EXPECT_EQ(parsed["person"]["age"], 30);
    EXPECT_EQ(parsed["person"]["address"]["street"], "123 Main St");
    EXPECT_EQ(parsed["person"]["address"]["city"], "New York");
    EXPECT_EQ(parsed["person"]["address"]["zip"], "10001");
}


TEST_F(JsonParserTest, ErrorHandling) {
    
    std::string invalidJson = R"({"name":"John","age":30,})"; 
    
    
    EXPECT_THROW(json::parse(invalidJson), json::parse_error);
    
    
    try {
        json parsed = json::parse(invalidJson);
        FAIL() << "Expected json::parse_error";
    } catch (const json::parse_error& e) {
        
        SUCCEED();
    } catch (...) {
        FAIL() << "Expected json::parse_error, got different exception";
    }
}
