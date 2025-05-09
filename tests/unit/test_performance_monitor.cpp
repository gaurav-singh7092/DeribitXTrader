#include <gtest/gtest.h>
#include "performance/monitor.h"
#include <chrono>
#include <thread>


class PerformanceMonitorTest : public ::testing::Test {
protected:
    void SetUp() override {
        
        getPerformanceMonitor().reset();
    }

    void TearDown() override {
        
        getPerformanceMonitor().reset();
    }
};


TEST_F(PerformanceMonitorTest, BasicMeasurement) {
    auto& monitor = getPerformanceMonitor();
    
    
    monitor.start_measurement(PerformanceMonitor::ORDER_EXECUTION);
    
    
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    
    monitor.stop_measurement(PerformanceMonitor::ORDER_EXECUTION);
    
    
    auto metrics = monitor.get_raw_metrics();
    
    
    EXPECT_EQ(metrics[PerformanceMonitor::ORDER_EXECUTION].size(), 1);
    
    
    EXPECT_TRUE(metrics[PerformanceMonitor::ORDER_EXECUTION][0].is_complete);
    
    
    EXPECT_GT(metrics[PerformanceMonitor::ORDER_EXECUTION][0].elapsed_time.count(), 0);
}


TEST_F(PerformanceMonitorTest, MultipleMeasurements) {
    auto& monitor = getPerformanceMonitor();
    
    
    monitor.start_measurement(PerformanceMonitor::ORDER_EXECUTION);
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    monitor.stop_measurement(PerformanceMonitor::ORDER_EXECUTION);
    
    monitor.start_measurement(PerformanceMonitor::MARKET_DATA_HANDLING);
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    monitor.stop_measurement(PerformanceMonitor::MARKET_DATA_HANDLING);
    
    monitor.start_measurement(PerformanceMonitor::WEBSOCKET_COMMUNICATION);
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    monitor.stop_measurement(PerformanceMonitor::WEBSOCKET_COMMUNICATION);
    
    
    auto metrics = monitor.get_raw_metrics();
    
    
    EXPECT_EQ(metrics[PerformanceMonitor::ORDER_EXECUTION].size(), 1);
    EXPECT_EQ(metrics[PerformanceMonitor::MARKET_DATA_HANDLING].size(), 1);
    EXPECT_EQ(metrics[PerformanceMonitor::WEBSOCKET_COMMUNICATION].size(), 1);
    
    
    EXPECT_TRUE(metrics[PerformanceMonitor::ORDER_EXECUTION][0].is_complete);
    EXPECT_TRUE(metrics[PerformanceMonitor::MARKET_DATA_HANDLING][0].is_complete);
    EXPECT_TRUE(metrics[PerformanceMonitor::WEBSOCKET_COMMUNICATION][0].is_complete);
}


TEST_F(PerformanceMonitorTest, UniqueIdMeasurements) {
    auto& monitor = getPerformanceMonitor();
    
    
    monitor.start_measurement(PerformanceMonitor::ORDER_EXECUTION, "order1");
    monitor.start_measurement(PerformanceMonitor::ORDER_EXECUTION, "order2");
    
    
    monitor.stop_measurement(PerformanceMonitor::ORDER_EXECUTION, "order2");
    monitor.stop_measurement(PerformanceMonitor::ORDER_EXECUTION, "order1");
    
    
    auto metrics = monitor.get_raw_metrics();
    
    
    EXPECT_EQ(metrics[PerformanceMonitor::ORDER_EXECUTION].size(), 2);
    
    
    EXPECT_TRUE(metrics[PerformanceMonitor::ORDER_EXECUTION][0].is_complete);
    EXPECT_TRUE(metrics[PerformanceMonitor::ORDER_EXECUTION][1].is_complete);
}


TEST_F(PerformanceMonitorTest, ResetFunctionality) {
    auto& monitor = getPerformanceMonitor();
    
    
    monitor.start_measurement(PerformanceMonitor::ORDER_EXECUTION);
    monitor.stop_measurement(PerformanceMonitor::ORDER_EXECUTION);
    
    
    auto metrics = monitor.get_raw_metrics();
    EXPECT_EQ(metrics[PerformanceMonitor::ORDER_EXECUTION].size(), 1);
    
    
    monitor.reset();
    
    
    metrics = monitor.get_raw_metrics();
    EXPECT_EQ(metrics[PerformanceMonitor::ORDER_EXECUTION].size(), 0);
}


TEST_F(PerformanceMonitorTest, ReportGeneration) {
    auto& monitor = getPerformanceMonitor();
    
    
    for (int i = 0; i < 10; i++) {
        monitor.start_measurement(PerformanceMonitor::ORDER_EXECUTION);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        monitor.stop_measurement(PerformanceMonitor::ORDER_EXECUTION);
    }
    
    
    std::string report = monitor.generate_report();
    
    
    EXPECT_FALSE(report.empty());
    
    
    EXPECT_NE(report.find("Order Execution"), std::string::npos);
    EXPECT_NE(report.find("Mean:"), std::string::npos);
    EXPECT_NE(report.find("Min:"), std::string::npos);
    EXPECT_NE(report.find("Max:"), std::string::npos);
}
