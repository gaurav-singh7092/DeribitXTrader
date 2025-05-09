#include "latency/tracker.h"
#include "utils/utils.h"
using namespace std;
void PerformanceMonitor::start_measurement(MeasurementType type, const string& unique_id) {
    lock_guard<mutex> lock(metrics_mutex);
    TimingData metric;
    metric.begin_time = chrono::high_resolution_clock::now();
    if (unique_id.empty()) {
        performance_data[type].push_back(metric);
    } else {
        ongoing_measurements[unique_id] = metric;
    }
}
void PerformanceMonitor::stop_measurement(MeasurementType type, const string& unique_id) {
    lock_guard<mutex> lock(metrics_mutex);
    auto finish_time = chrono::high_resolution_clock::now();
    if (unique_id.empty()) {
        for (auto& metric : performance_data[type]) {
            if (!metric.is_complete) {
                metric.finish_time = finish_time;
                metric.elapsed_time = chrono::duration_cast<chrono::nanoseconds>(
                    metric.finish_time - metric.begin_time
                );
                metric.is_complete = true;
                break;
            }
        }
    } else {
        auto it = ongoing_measurements.find(unique_id);
        if (it != ongoing_measurements.end()) {
            it->second.finish_time = finish_time;
            it->second.elapsed_time = chrono::duration_cast<chrono::nanoseconds>(
                it->second.finish_time - it->second.begin_time
            );
            it->second.is_complete = true;
            performance_data[type].push_back(it->second);
            ongoing_measurements.erase(it);
        }
    }
}
string PerformanceMonitor::generate_report() {
    lock_guard<mutex> lock(metrics_mutex);
    int terminal_width = utils::getTerminalWidth();
    ostringstream report;
    const string reset_color = "\033[0m";
    const string header_color = "\033[1;36m"; 
    const string section_color = "\033[1;32m"; 
    const string metric_color = "\033[1;33m"; 
    const string footer_color = "\033[1;34m"; 
    string header = "Performance Benchmarking Report";
    int padding_length = (terminal_width - header.length()) / 2;
    string padding(padding_length, '=');
    report << header_color << padding << header << padding << reset_color << "\n\n";
    const char* type_names[] = {
        "Order Execution",
        "Market Data Handling", 
        "WebSocket Communication", 
        "Trading Cycle Full"
    };
    int type_col_width = 30;
    int metric_col_width = (terminal_width - type_col_width - 4) / 2;
    for (int type = 0; type < 4; ++type) {
        auto metrics = performance_data[static_cast<MeasurementType>(type)];
        if (metrics.empty()) continue;
        vector<chrono::nanoseconds> durations;
        for (const auto& metric : metrics) {
            if (metric.is_complete) {
                durations.push_back(metric.elapsed_time);
            }
        }
        if (durations.empty()) {
            report << section_color << type_names[type] << reset_color
                   << " Latency: No completed measurements\n\n";
            continue;
        }
        sort(durations.begin(), durations.end());
        auto total_measurements = durations.size();
        auto mean_duration = accumulate(durations.begin(), durations.end(), 
            chrono::nanoseconds(0)) / total_measurements;
        auto percentile_50 = durations[total_measurements * 0.5];
        auto percentile_90 = durations[total_measurements * 0.9];
        auto percentile_99 = durations[total_measurements * 0.99];
        auto min_duration = durations.front();
        auto max_duration = durations.back();
        report << section_color << left << setw(type_col_width) << type_names[type] 
               << reset_color
               << right 
               << fixed << setprecision(3);
        report << "  " << metric_color << "Meas: " << reset_color << setw(6) << total_measurements 
               << "  " << metric_color << "Mean: " << reset_color << setw(8) << mean_duration.count() / 1000.0 << " µs\n";
        report << string(type_col_width, ' ');
        report << "  " << metric_color << "Min:  " << reset_color << setw(8) << min_duration.count() / 1000.0 << " µs"
               << "  " << metric_color << "Max:  " << reset_color << setw(8) << max_duration.count() / 1000.0 << " µs\n";
        report << string(type_col_width, ' ')
               << "  " << metric_color << "50th: " << reset_color << setw(8) << percentile_50.count() / 1000.0 << " µs"
               << "  " << metric_color << "90th: " << reset_color << setw(8) << percentile_90.count() / 1000.0 << " µs"
               << "  " << metric_color << "99th: " << reset_color << setw(8) << percentile_99.count() / 1000.0 << " µs\n\n";
    }
    report << footer_color << string(terminal_width, '=') << reset_color << "\n";
    return report.str();
}
map<PerformanceMonitor::MeasurementType, vector<PerformanceMonitor::TimingData>> PerformanceMonitor::get_raw_metrics() {
    lock_guard<mutex> lock(metrics_mutex);
    return performance_data;
}
void PerformanceMonitor::reset() {
    lock_guard<mutex> lock(metrics_mutex);
    performance_data.clear();
    ongoing_measurements.clear();
    int terminal_width = utils::getTerminalWidth();
    const string message = "Performance metrics have been reset.";
    int padding_length = (terminal_width - message.length()) / 2;
    string padding(padding_length, ' ');
    cout << padding << message << endl;
}
PerformanceMonitor& getPerformanceMonitor() {
    static PerformanceMonitor monitor;
    return monitor;
}