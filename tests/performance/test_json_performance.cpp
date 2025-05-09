#include <gtest/gtest.h>
#include <chrono>
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <nlohmann/json.hpp>
#include "data_format/json_parser.hpp"

using json = nlohmann::json;
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


class JsonPerformanceTest : public ::testing::Test {
protected:
    
    std::string small_json_str;
    std::string medium_json_str;
    std::string large_json_str;

    
    const int small_iterations = 10000;
    const int medium_iterations = 1000;
    const int large_iterations = 100;

    void SetUp() override {
        
        small_json_str = R"({
            "name": "John Doe",
            "age": 30,
            "email": "john.doe@example.com",
            "is_active": true
        })";

        
        medium_json_str = R"({
            "users": [
                {"id": 1, "name": "User 1", "email": "user1@example.com", "roles": ["admin", "user"]},
                {"id": 2, "name": "User 2", "email": "user2@example.com", "roles": ["user"]},
                {"id": 3, "name": "User 3", "email": "user3@example.com", "roles": ["user", "editor"]},
                {"id": 4, "name": "User 4", "email": "user4@example.com", "roles": ["user"]},
                {"id": 5, "name": "User 5", "email": "user5@example.com", "roles": ["user", "moderator"]}
            ],
            "metadata": {
                "total_count": 5,
                "active_count": 5,
                "page": 1,
                "per_page": 10,
                "total_pages": 1
            }
        })";

        
        large_json_str = R"({
            "company": {
                "name": "Example Corp",
                "founded": 2005,
                "active": true,
                "address": {
                    "street": "123 Main St",
                    "city": "Anytown",
                    "state": "CA",
                    "zip": "12345",
                    "country": "USA",
                    "coordinates": {
                        "latitude": 37.7749,
                        "longitude": -122.4194
                    }
                },
                "employees": [
                    {"id": 1, "name": "Employee 1", "department": "Engineering", "skills": ["C++", "Python", "JavaScript"]},
                    {"id": 2, "name": "Employee 2", "department": "Marketing", "skills": ["SEO", "Content", "Analytics"]},
                    {"id": 3, "name": "Employee 3", "department": "Sales", "skills": ["Negotiation", "Communication"]},
                    {"id": 4, "name": "Employee 4", "department": "Engineering", "skills": ["Java", "Kotlin", "Swift"]},
                    {"id": 5, "name": "Employee 5", "department": "HR", "skills": ["Recruiting", "Training"]},
                    {"id": 6, "name": "Employee 6", "department": "Engineering", "skills": ["C++", "Rust", "Go"]},
                    {"id": 7, "name": "Employee 7", "department": "Finance", "skills": ["Accounting", "Budgeting"]},
                    {"id": 8, "name": "Employee 8", "department": "Engineering", "skills": ["Python", "Django", "Flask"]},
                    {"id": 9, "name": "Employee 9", "department": "Product", "skills": ["UX", "UI", "Research"]},
                    {"id": 10, "name": "Employee 10", "department": "Engineering", "skills": ["C++", "CMake", "Git"]}
                ],
                "departments": ["Engineering", "Marketing", "Sales", "HR", "Finance", "Product"],
                "projects": [
                    {
                        "id": "proj-1",
                        "name": "Project Alpha",
                        "status": "active",
                        "start_date": "2023-01-15",
                        "end_date": "2023-06-30",
                        "team_members": [1, 4, 6, 8, 10],
                        "milestones": [
                            {"name": "Planning", "completed": true, "completion_date": "2023-02-01"},
                            {"name": "Development", "completed": true, "completion_date": "2023-04-15"},
                            {"name": "Testing", "completed": false, "completion_date": null},
                            {"name": "Deployment", "completed": false, "completion_date": null}
                        ],
                        "budget": {
                            "allocated": 150000,
                            "spent": 75000,
                            "remaining": 75000,
                            "currency": "USD"
                        }
                    },
                    {
                        "id": "proj-2",
                        "name": "Project Beta",
                        "status": "planning",
                        "start_date": "2023-07-01",
                        "end_date": "2023-12-31",
                        "team_members": [2, 3, 5, 9],
                        "milestones": [
                            {"name": "Planning", "completed": false, "completion_date": null},
                            {"name": "Development", "completed": false, "completion_date": null},
                            {"name": "Testing", "completed": false, "completion_date": null},
                            {"name": "Deployment", "completed": false, "completion_date": null}
                        ],
                        "budget": {
                            "allocated": 200000,
                            "spent": 0,
                            "remaining": 200000,
                            "currency": "USD"
                        }
                    }
                ],
                "financial_data": {
                    "years": [2020, 2021, 2022],
                    "revenue": [1500000, 1800000, 2200000],
                    "expenses": [1200000, 1400000, 1700000],
                    "profit": [300000, 400000, 500000],
                    "growth_rate": [null, 20, 22.22],
                    "currency": "USD"
                }
            }
        })";
    }
};


TEST_F(JsonPerformanceTest, ParsingPerformance) {
    
    {
        PerformanceTimer timer("Small JSON Parsing", small_iterations);
        for (int i = 0; i < small_iterations; i++) {
            json parsed = json::parse(small_json_str);
            ASSERT_EQ(parsed["name"], "John Doe");
        }
    }

    
    {
        PerformanceTimer timer("Medium JSON Parsing", medium_iterations);
        for (int i = 0; i < medium_iterations; i++) {
            json parsed = json::parse(medium_json_str);
            ASSERT_EQ(parsed["users"].size(), 5);
        }
    }

    
    {
        PerformanceTimer timer("Large JSON Parsing", large_iterations);
        for (int i = 0; i < large_iterations; i++) {
            json parsed = json::parse(large_json_str);
            ASSERT_EQ(parsed["company"]["employees"].size(), 10);
        }
    }
}


TEST_F(JsonPerformanceTest, SerializationPerformance) {
    json small_json = json::parse(small_json_str);
    json medium_json = json::parse(medium_json_str);
    json large_json = json::parse(large_json_str);

    
    {
        PerformanceTimer timer("Small JSON Serialization", small_iterations);
        for (int i = 0; i < small_iterations; i++) {
            std::string serialized = small_json.dump();
            ASSERT_FALSE(serialized.empty());
        }
    }

    
    {
        PerformanceTimer timer("Medium JSON Serialization", medium_iterations);
        for (int i = 0; i < medium_iterations; i++) {
            std::string serialized = medium_json.dump();
            ASSERT_FALSE(serialized.empty());
        }
    }

    
    {
        PerformanceTimer timer("Large JSON Serialization", large_iterations);
        for (int i = 0; i < large_iterations; i++) {
            std::string serialized = large_json.dump();
            ASSERT_FALSE(serialized.empty());
        }
    }
}


TEST_F(JsonPerformanceTest, ManipulationPerformance) {
    
    {
        PerformanceTimer timer("JSON Object Manipulation", small_iterations);
        for (int i = 0; i < small_iterations; i++) {
            json obj;
            obj["id"] = i;
            obj["name"] = "Test User";
            obj["email"] = "test" + std::to_string(i) + "@example.com";
            obj["active"] = true;
            obj["score"] = 95.5;
            obj["tags"] = json::array({"tag1", "tag2", "tag3"});

            
            obj["name"] = "Modified User";
            obj["score"] = 98.7;
            obj["tags"].push_back("tag4");

            
            std::string name = obj["name"];
            double score = obj["score"];
            bool active = obj["active"];

            ASSERT_EQ(name, "Modified User");
            ASSERT_EQ(score, 98.7);
            ASSERT_TRUE(active);
        }
    }
}


TEST_F(JsonPerformanceTest, DeepAccessPerformance) {
    json large_json = json::parse(large_json_str);

    
    {
        PerformanceTimer timer("JSON Deep Access", medium_iterations);
        for (int i = 0; i < medium_iterations; i++) {
            
            std::string company_name = large_json["company"]["name"];
            std::string street = large_json["company"]["address"]["street"];
            double latitude = large_json["company"]["address"]["coordinates"]["latitude"];

            
            std::string dept = large_json["company"]["departments"][0];
            std::string emp_name = large_json["company"]["employees"][0]["name"];
            std::string skill = large_json["company"]["employees"][0]["skills"][0];

            
            double allocated_budget = large_json["company"]["projects"][0]["budget"]["allocated"];
            std::string milestone = large_json["company"]["projects"][0]["milestones"][0]["name"];

            ASSERT_EQ(company_name, "Example Corp");
            ASSERT_EQ(street, "123 Main St");
            ASSERT_EQ(latitude, 37.7749);
            ASSERT_EQ(dept, "Engineering");
            ASSERT_EQ(emp_name, "Employee 1");
            ASSERT_EQ(skill, "C++");
            ASSERT_EQ(allocated_budget, 150000);
            ASSERT_EQ(milestone, "Planning");
        }
    }
}


TEST_F(JsonPerformanceTest, ArrayOperationsPerformance) {
    
    {
        PerformanceTimer timer("JSON Array Operations", medium_iterations);
        for (int i = 0; i < medium_iterations; i++) {
            
            json arr = json::array();

            
            for (int j = 0; j < 100; j++) {
                json obj;
                obj["id"] = j;
                obj["value"] = j * 2;
                arr.push_back(obj);
            }

            
            int sum = 0;
            for (const auto& item : arr) {
                sum += item["value"].get<int>();
            }

            
            json filtered = json::array();
            for (const auto& item : arr) {
                if (item["id"].get<int>() % 2 == 0) {
                    filtered.push_back(item);
                }
            }

            ASSERT_EQ(arr.size(), 100);
            ASSERT_EQ(filtered.size(), 50);
            ASSERT_EQ(sum, 9900); 
        }
    }
}


int main(int argc, char **argv) {
    std::cout << "===== Performance Tests =====" << std::endl;
    std::cout << "Running JSON, WebSocket, and Market API performance tests...\n" << std::endl;
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
