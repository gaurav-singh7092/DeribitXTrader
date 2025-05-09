#ifndef PERFORMANCE_MONITOR_H
#define PERFORMANCE_MONITOR_H
#include <chrono>
#include <map>
#include <vector>
#include <mutex>
#include <string>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <iomanip>
using namespace std;
class PerformanceMonitor {
public:
    enum MeasurementType {
        ORDER_EXECUTION,
        MARKET_DATA_HANDLING,
        WEBSOCKET_COMMUNICATION,
        TRADING_CYCLE_FULL
    };
    struct TimingData {
        chrono::high_resolution_clock::time_point begin_time;
        chrono::high_resolution_clock::time_point finish_time;
        chrono::nanoseconds elapsed_time{0};
        bool is_complete{false};
    };
    void start_measurement(MeasurementType type, const string& unique_id = "");
    void stop_measurement(MeasurementType type, const string& unique_id = "");
    string generate_report();
    map<MeasurementType, vector<TimingData>> get_raw_metrics();
    void reset();
private:
    mutex metrics_mutex;
    map<MeasurementType, vector<TimingData>> performance_data;
    map<string, TimingData> ongoing_measurements;
};
PerformanceMonitor& getPerformanceMonitor();
#endif 
