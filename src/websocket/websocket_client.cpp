#include "websocket/websocket_client.h"
#include "utils/utils.h"
#include "authentication/password.h"
#include <fmt/color.h>
#include "latency/tracker.h"
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
using namespace std;
bool isDataStreaming = false;
extern bool AUTHENTICATION_SENT; 
ConnectionDetails::ConnectionDetails(
    int id, 
    string uri, 
    SocketEndpoint* endpoint
) :
    m_connection_id(id),
    m_connection_status("Connecting"),
    m_endpoint_uri(uri),
    m_server_info("N/A"),
    m_received_data({}),
    m_transaction_logs({}),
    m_endpoint_controller(endpoint),
    DATA_PROCESSED(false),
    m_webSocketClient(std::make_unique<ix::WebSocket>())
{
    setup_websocket();
}
ConnectionDetails::~ConnectionDetails() {
    if (m_webSocketClient) {
        m_webSocketClient->stop();
    }
}
void ConnectionDetails::setup_websocket() {
    m_webSocketClient->setUrl(m_endpoint_uri);
    m_webSocketClient->setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Message) {
            getPerformanceMonitor().start_measurement(
                PerformanceMonitor::WEBSOCKET_COMMUNICATION, 
                "websocket_message_" + to_string(m_connection_id)
            );
            try {
                string payload = msg->str;
                json received_json;
                try {
                    received_json = json::parse(payload);
                } catch (const json::parse_error& e) {
                    cerr << "JSON parse error: " << e.what() << endl;
                    cerr << "Problematic payload: " << payload << endl;
                    return;
                }
                if (received_json.contains("method")) {
                    string method = received_json.value("method", "");
                    if (method == "subscription" && isDataStreaming) {
                        auto params = received_json.value("params", json{});
                        auto data = params.value("data", json{});
                        if (!data.is_null() && data.is_object()) {
                            static vector<double> priceHistory;
                            static string currentInstrument = "";
                            static double previousPrice = 0.0;
                            static double highPrice = 0.0;
                            static double lowPrice = std::numeric_limits<double>::max();
                            static double openPrice = 0.0;
                            static int updateCount = 0;
                            utils::clear_console();
                            int terminal_width = utils::getTerminalWidth();
                            string separator(terminal_width, '-');
                            string thin_separator(terminal_width, '.');
                            fmt::print(fg(fmt::rgb(0, 120, 212)) | bg(fmt::rgb(20, 20, 30)) | fmt::emphasis::bold,
                                "{:^{}}\n", "💹 LIVE MARKET DATA STREAM", terminal_width);
                            fmt::print(fg(fmt::rgb(100, 100, 120)), "{}\n", separator);
                            if (data.contains("price") && data["price"].is_number() &&
                                data.contains("timestamp") && data["timestamp"].is_number() &&
                                data.contains("index_name") && data["index_name"].is_string()) {
                                double price = data["price"];
                                int64_t timestamp = data["timestamp"];
                                string index_name = data["index_name"];
                                if (currentInstrument != index_name) {
                                    currentInstrument = index_name;
                                    priceHistory.clear();
                                    previousPrice = price;
                                    highPrice = price;
                                    lowPrice = price;
                                    openPrice = price;
                                    updateCount = 0;
                                }
                                updateCount++;
                                highPrice = max(highPrice, price);
                                lowPrice = min(lowPrice, price);
                                if (priceHistory.size() >= 30) priceHistory.erase(priceHistory.begin());
                                priceHistory.push_back(price);
                                double priceChange = price - previousPrice;
                                double percentChange = previousPrice != 0 ? (priceChange / previousPrice) * 100 : 0;
                                time_t t = timestamp / 1000; 
                                char time_buf[64];
                                strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", localtime(&t));
                                fmt::print(fg(fmt::rgb(255, 255, 255)) | bg(fmt::rgb(40, 44, 52)) | fmt::emphasis::bold,
                                    " 🎯 Instrument: ");
                                fmt::print(fg(fmt::rgb(255, 215, 0)) | bg(fmt::rgb(40, 44, 52)) | fmt::emphasis::bold,
                                    "{:<30} ", index_name);
                                fmt::print(fg(fmt::rgb(255, 255, 255)) | bg(fmt::rgb(40, 44, 52)) | fmt::emphasis::bold,
                                    "⏰ Time: ");
                                fmt::print(fg(fmt::rgb(120, 200, 255)) | bg(fmt::rgb(40, 44, 52)),
                                    "{}\n", time_buf);
                                fmt::print(fg(fmt::rgb(100, 100, 120)), "{}\n", thin_separator);
                                fmt::print(fg(fmt::rgb(255, 255, 255)) | fmt::emphasis::bold,
                                    " 💰 PRICE: ");
                                auto priceColor = priceChange >= 0 ? 
                                                fmt::rgb(0, 255, 127) : 
                                                fmt::rgb(255, 69, 0);
                                fmt::print(fg(priceColor) | fmt::emphasis::bold,
                                    "${:.2f} ", price);
                                string changeArrow = priceChange >= 0 ? "▲" : "▼";
                                fmt::print(fg(priceColor) | fmt::emphasis::bold,
                                    "{} ${:.2f} ({:.2f}%)\n", 
                                    changeArrow, fabs(priceChange), percentChange);
                                fmt::print(fg(fmt::rgb(255, 255, 255)) | fmt::emphasis::bold,
                                    " 📊 STATS: ");
                                fmt::print(fg(fmt::rgb(200, 200, 200)),
                                    "Open: ");
                                fmt::print(fg(fmt::rgb(100, 200, 255)) | fmt::emphasis::bold,
                                    "${:.2f} ", openPrice);
                                fmt::print(fg(fmt::rgb(200, 200, 200)),
                                    "High: ");
                                fmt::print(fg(fmt::rgb(0, 255, 127)) | fmt::emphasis::bold,
                                    "${:.2f} ", highPrice);
                                fmt::print(fg(fmt::rgb(200, 200, 200)),
                                    "Low: ");
                                fmt::print(fg(fmt::rgb(255, 69, 0)) | fmt::emphasis::bold,
                                    "${:.2f} ", lowPrice);
                                fmt::print(fg(fmt::rgb(200, 200, 200)),
                                    "Updates: ");
                                fmt::print(fg(fmt::rgb(255, 215, 0)) | fmt::emphasis::bold,
                                    "{}\n", updateCount);
                                fmt::print(fg(fmt::rgb(100, 100, 120)), "{}\n", thin_separator);
                                fmt::print(fg(fmt::rgb(255, 255, 255)) | fmt::emphasis::bold,
                                    " 📊 MARKET SUMMARY:\n\n");
                                string trend_indicator;
                                fmt::rgb trend_color;
                                string trend_label;
                                if (priceHistory.size() >= 5) {
                                    int up_count = 0;
                                    int down_count = 0;
                                    for (size_t i = priceHistory.size() - 5; i < priceHistory.size() - 1; ++i) {
                                        if (priceHistory[i+1] > priceHistory[i]) up_count++;
                                        else if (priceHistory[i+1] < priceHistory[i]) down_count++;
                                    }
                                    if (up_count > down_count) {
                                        trend_indicator = "↗️  BULLISH";
                                        trend_color = fmt::rgb(0, 255, 127);
                                        trend_label = "Market trending upward";
                                    } else if (down_count > up_count) {
                                        trend_indicator = "↘️  BEARISH";
                                        trend_color = fmt::rgb(255, 69, 0);
                                        trend_label = "Market trending downward";
                                    } else {
                                        trend_indicator = "↔️  SIDEWAYS";
                                        trend_color = fmt::rgb(255, 215, 0);
                                        trend_label = "Market moving sideways";
                                    }
                                } else {
                                    trend_indicator = "❓ WAITING";
                                    trend_color = fmt::rgb(150, 150, 150);
                                    trend_label = "Collecting data...";
                                }
                                fmt::print(fg(fmt::rgb(200, 200, 200)),
                                    " Market Trend: ");
                                fmt::print(fg(trend_color) | fmt::emphasis::bold,
                                    "{} ", trend_indicator);
                                fmt::print(fg(fmt::rgb(180, 180, 180)) | fmt::emphasis::italic,
                                    "- {}\n", trend_label);
                                double volatility = 0.0;
                                if (priceHistory.size() >= 10) {
                                    double sum = 0.0;
                                    for (size_t i = priceHistory.size() - 10; i < priceHistory.size() - 1; ++i) {
                                        sum += fabs(priceHistory[i+1] - priceHistory[i]);
                                    }
                                    volatility = sum / 9.0; 
                                }
                                string volatility_level;
                                fmt::rgb volatility_color;
                                if (volatility < 0.0001 * price) {
                                    volatility_level = "LOW";
                                    volatility_color = fmt::rgb(0, 255, 127);
                                } else if (volatility < 0.001 * price) {
                                    volatility_level = "MEDIUM";
                                    volatility_color = fmt::rgb(255, 215, 0);
                                } else {
                                    volatility_level = "HIGH";
                                    volatility_color = fmt::rgb(255, 69, 0);
                                }
                                fmt::print(fg(fmt::rgb(200, 200, 200)),
                                    " Volatility: ");
                                fmt::print(fg(volatility_color) | fmt::emphasis::bold,
                                    "{}", volatility_level);
                                fmt::print(" (${:.6f} avg change)\n", volatility);
                                double session_change = price - openPrice;
                                double session_percent = openPrice != 0 ? (session_change / openPrice) * 100 : 0;
                                fmt::rgb session_color = session_change >= 0 ? fmt::rgb(0, 255, 127) : fmt::rgb(255, 69, 0);
                                fmt::print(fg(fmt::rgb(200, 200, 200)),
                                    " Session Change: ");
                                fmt::print(fg(session_color) | fmt::emphasis::bold,
                                    "${:.2f} ({:.2f}%)\n", session_change, session_percent);
                                fmt::print(fg(fmt::rgb(200, 200, 200)),
                                    " Price Range: ");
                                fmt::print("${:.2f} - ${:.2f} (${:.2f})\n", 
                                           lowPrice, highPrice, highPrice - lowPrice);
                                fmt::print(fg(fmt::rgb(200, 200, 200)),
                                    " Data Points: ");
                                fmt::print(fg(fmt::rgb(255, 215, 0)) | fmt::emphasis::bold,
                                    "{}\n", updateCount);
                                fmt::print(fg(fmt::rgb(100, 100, 120)), "{}\n", separator);
                                fmt::print(fg(fmt::rgb(180, 180, 180)) | fmt::emphasis::italic,
                                    " Press 'q' to stop streaming\n");
                                previousPrice = price;
                            } else {
                                cerr << "Unexpected data format" << endl;
                            }
                        } else {
                            cerr << "Invalid or null data received" << endl;
                        }
                    }
                }
                if (!isDataStreaming) {
                    m_received_data.push_back("RECEIVED: " + payload);
                    record_summary(payload, "RECEIVED");
                    if (received_json.contains("id") && received_json.contains("result")) {
                        if (!m_received_data.empty() && m_received_data.size() >= 2) {
                            string prev_message = m_received_data[m_received_data.size() - 2]; 
                            if (prev_message.find("public/get_order_book") != string::npos) {
                                json req_json;
                                try {
                                    size_t json_start = prev_message.find("{");
                                    if (json_start != string::npos) {
                                        req_json = json::parse(prev_message.substr(json_start));
                                        if (req_json.contains("params") && req_json["params"].contains("instrument_name")) {
                                            string instrument = req_json["params"]["instrument_name"];
                                            int depth = req_json["params"].contains("depth") ? 
                                                       req_json["params"]["depth"].get<int>() : 10;
                                            utils::printOrderbook(instrument, payload, depth);
                                        }
                                    }
                                } catch (const json::parse_error& e) {
                                }
                            }
                            else if (prev_message.find("private/get_positions") != string::npos) {
                                utils::printPositions(payload);
                            }
                            else if (prev_message.find("private/get_open_orders") != string::npos) {
                                utils::printOpenOrders(payload);
                            }
                        }
                    }
                    AUTHENTICATION_SENT = false;
                }
                DATA_PROCESSED = true;
                connection_cv.notify_one();
            }
            catch (const exception& e) {
                cerr << "Error processing message: " << e.what() << endl;
                DATA_PROCESSED = true;
                connection_cv.notify_one();
            }
            getPerformanceMonitor().stop_measurement(
                PerformanceMonitor::WEBSOCKET_COMMUNICATION, 
                "websocket_message_" + to_string(m_connection_id)
            );
        }
        else if (msg->type == ix::WebSocketMessageType::Open) {
            m_connection_status = "Connected";
            m_server_info = "IXWebSocket";
        }
        else if (msg->type == ix::WebSocketMessageType::Error) {
            m_connection_status = "Error";
            m_error_message = msg->errorInfo.reason;
            stringstream ss;
            ss << "Error: " << msg->errorInfo.reason;
            if (msg->errorInfo.http_status != 0) {
                ss << " HTTP Status: " << msg->errorInfo.http_status;
            }
            cerr << ss.str() << endl;
        }
        else if (msg->type == ix::WebSocketMessageType::Close) {
            m_connection_status = "Closed";
            stringstream ss;
            ss << "Close code: " << msg->closeInfo.code << ", reason: " << msg->closeInfo.reason;
            m_error_message = ss.str();
        }
    });
}
int ConnectionDetails::get_id() { return m_connection_id; }
string ConnectionDetails::get_status() { return m_connection_status; }
void ConnectionDetails::record_sent_message(string const &message) {
    m_received_data.push_back("SENT: " + message);
}
void ConnectionDetails::record_summary(string const &message, string const &sent) {
    if (message == "") return;
    json parsed_msg = json::parse(message);
    string cmd = parsed_msg.contains("method") ? parsed_msg["method"] : "received";
    map<string, string> summary;
    map<string, function<map<string, string>(json)>> action_map = 
    {
        {"public/auth", [](json parsed_msg){ 
            map<string, string> summary;
            summary["method"] = parsed_msg["method"];
            summary["grant_type"] = parsed_msg["params"]["grant_type"];
            summary["client_id"] = parsed_msg["params"]["client_id"];
            summary["timestamp"] = to_string(parsed_msg["params"]["timestamp"].get<long long>());
            summary["nonce"] = parsed_msg["params"]["nonce"];
            summary["scope"] = parsed_msg["params"]["scope"];
            return summary;
        }},
        {"private/sell", [](json parsed_msg){
            map<string, string> summary = {};
            summary["method"] = parsed_msg["method"];
            summary["instrument_name"] = parsed_msg["params"]["instrument_name"];
            summary["access_token"] = parsed_msg["params"]["access_token"];
            if (parsed_msg["params"].contains("amount"))
                summary["amount"] = to_string(parsed_msg["params"]["amount"].get<double>());
            if (parsed_msg["params"].contains("contracts"))
                summary["contracts"] = to_string(parsed_msg["params"]["contracts"].get<int>());
            summary["order_type"] = parsed_msg["params"]["type"];
            summary["label"] = parsed_msg["params"]["label"];
            summary["time_in_force"] = parsed_msg["params"]["time_in_force"];
            if (parsed_msg["params"].contains("price"))
                summary["price"] = to_string(parsed_msg["params"]["price"].get<double>());
            return summary;
        }},
        {"private/buy", [](json parsed_msg){
            map<string, string> summary = {};
            summary["method"] = parsed_msg["method"];
            summary["instrument_name"] = parsed_msg["params"]["instrument_name"];
            summary["access_token"] = parsed_msg["params"]["access_token"];
            if (parsed_msg["params"].contains("amount"))
                summary["amount"] = to_string(parsed_msg["params"]["amount"].get<double>());
            if (parsed_msg["params"].contains("contracts"))
                summary["contracts"] = to_string(parsed_msg["params"]["contracts"].get<int>());
            summary["order_type"] = parsed_msg["params"]["type"];
            summary["label"] = parsed_msg["params"]["label"];
            summary["time_in_force"] = parsed_msg["params"]["time_in_force"];
            if (parsed_msg["params"].contains("price"))
                summary["price"] = to_string(parsed_msg["params"]["price"].get<double>());
            return summary;
        }},
        {"private/edit", [](json parsed_msg){
            map<string, string> summary = {};
            summary["method"] = parsed_msg["method"];
            summary["order_id"] = parsed_msg["params"]["order_id"];
            if (parsed_msg["params"].contains("amount"))
                summary["new_amount"] = to_string(parsed_msg["params"]["amount"].get<double>());
            if (parsed_msg["params"].contains("price"))
                summary["new_price"] = to_string(parsed_msg["params"]["price"].get<double>());
            return summary;
        }},
        {"private/cancel", [](json parsed_msg){
            map<string, string> summary = {};
            summary["method"] = parsed_msg["method"];
            summary["order_id"] = parsed_msg["params"]["order_id"];
            return summary;
        }},
        {"private/cancel_all", [](json parsed_msg){
            map<string, string> summary = {};
            summary["method"] = parsed_msg["method"];
            return summary;
        }},
        {"private/cancel_all_by_instrument", [](json parsed_msg){
            map<string, string> summary = {};
            summary["method"] = parsed_msg["method"];
            summary["instrument"] = parsed_msg["params"]["instrument"];
            return summary;
        }},
        {"private/cancel_by_label", [](json parsed_msg){
            map<string, string> summary = {};
            summary["method"] = parsed_msg["method"];
            summary["label"] = parsed_msg["params"]["label"];
            return summary;
        }},
        {"private/cancel_all_by_currency", [](json parsed_msg){
            map<string, string> summary = {};
            summary["method"] = parsed_msg["method"];
            summary["currency"] = parsed_msg["params"]["currency"];
            return summary;
        }},
        {"private/get_open_orders", [](json parsed_msg){
            map<string, string> summary = {};
            summary["method"] = parsed_msg["method"];
            return summary;
        }},
        {"private/get_open_orders_by_instrument", [](json parsed_msg){
            map<string, string> summary = {};
            summary["method"] = parsed_msg["method"];
            summary["instrument"] = parsed_msg["params"]["instrument"];
            return summary;
        }},
        {"private/get_open_orders_by_currency", [](json parsed_msg){
            map<string, string> summary = {};
            summary["method"] = parsed_msg["method"];
            summary["currency"] = parsed_msg["params"]["currency"];
            return summary;
        }},
        {"private/get_open_orders_by_label", [](json parsed_msg){
            map<string, string> summary = {};
            summary["method"] = parsed_msg["method"];
            summary["currency"] = parsed_msg["params"]["currency"];
            summary["label"] = parsed_msg["params"]["label"];
            return summary;
        }},
        {"private/get_positions", [](json parsed_msg){
            map<string, string> summary = {};
            summary["method"] = parsed_msg["method"];
            if (parsed_msg["params"].contains("currency"))
                summary["currency"] = parsed_msg["params"]["currency"];
            if (parsed_msg["params"].contains("kind"))
                summary["kind"] = parsed_msg["params"]["kind"];
            return summary;
        }},
        {"public/get_order_book", [](json parsed_msg){
            map<string, string> summary = {};
            summary["method"] = parsed_msg["method"];
            summary["instrument_name"] = parsed_msg["params"]["instrument_name"];
            summary["depth"] = to_string(parsed_msg["params"]["depth"].get<int>());
            return summary;
        }},
        {"received", [](json parsed_msg){
            map<string, string> summary = {};
            if (parsed_msg.contains("result"))
                summary = {{"result", parsed_msg["result"].dump()}};
            else if (parsed_msg.contains("error"))
                summary = {{"error message", parsed_msg["error"].dump()}};
            return summary;
        }}
    };
    auto find = action_map.find(cmd);
    if (find == action_map.end()) {
        summary["id"] = to_string(parsed_msg["id"].get<int>());
        if (sent == "SENT") summary["method"] = parsed_msg["method"];
    }
    else {
        summary = find->second(parsed_msg);
    }
    m_transaction_logs.push_back(sent + " : \n" + utils::mapToString(summary));
}
void ConnectionDetails::close(uint16_t code, const string& reason) {
    if (m_webSocketClient) {
        m_webSocketClient->close(code, reason);
    }
}
bool ConnectionDetails::send(const string& message) {
    if (!m_webSocketClient || m_connection_status != "Connected") {
        return false;
    }
    m_webSocketClient->send(message);
    record_sent_message(message);
    return true;
}
ix::WebSocket* ConnectionDetails::get_websocket() {
    return m_webSocketClient.get();
}
ostream &operator<< (ostream &out, ConnectionDetails const &data) {
    out << "> URI: " << data.m_endpoint_uri << "\n"
        << "> Status: " << data.m_connection_status << "\n"
        << "> Remote Server: " << (data.m_server_info.empty() ? "None Specified" : data.m_server_info) << "\n"
        << "> Error/close reason: " << (data.m_error_message.empty() ? "N/A" : data.m_error_message) << "\n"
        << "> Messages Processed: (" << data.m_received_data.size() << ") \n";
    vector<string>::const_iterator it;
    for (it = data.m_transaction_logs.begin(); it != data.m_transaction_logs.end(); ++it) {
        out << *it << "\n";
    }
    return out;
}
SocketEndpoint::SocketEndpoint(): m_next_id(0) {
    ix::initNetSystem();
}
SocketEndpoint::~SocketEndpoint() {
    for (connection_list::const_iterator it = m_active_connections.begin(); it != m_active_connections.end(); ++it) {
        if (it->second->get_status() != "Connected") {
            continue;
        }
        cout << "> Closing connection " << it->second->get_id() << endl;
        it->second->close();
    }
    ix::uninitNetSystem();
}
int SocketEndpoint::connect(string const &uri) {
    int new_id = m_next_id++;
    ConnectionDetails::ptr metadata_ptr(new ConnectionDetails(new_id, uri, this));
    m_active_connections[new_id] = metadata_ptr;
    metadata_ptr->get_websocket()->start();
    return new_id;
}
ConnectionDetails::ptr SocketEndpoint::get_metadata(int id) const {
    connection_list::const_iterator it = m_active_connections.find(id);
    if (it == m_active_connections.end()) {
        return ConnectionDetails::ptr(); 
    }
    return it->second;
}
void SocketEndpoint::close(int id, uint16_t code, string reason) {
    connection_list::iterator it = m_active_connections.find(id);
    if (it == m_active_connections.end()) {
        cout << "> No connection found with id " << id << endl;
        return;
    }
    it->second->close(code, reason);
}
int SocketEndpoint::send(int id, string message) {
    connection_list::iterator it = m_active_connections.find(id);
    if (it == m_active_connections.end()) {
        cout << "> No connection found with id " << id << endl;
        return -1;
    }
    if (!it->second->send(message)) {
        cout << "> Error sending message to connection " << id << endl;
        return -1;
    }
    return 0;
}
int SocketEndpoint::streamSubscriptions(const vector<string>& connections) {
    if (connections.empty()) {
        cout << "No subscriptions to stream." << endl;
        return -1;
    }
    json subscribe = {
        {"jsonrpc", "2.0"},
        {"id", 4235},
        {"method", "private/subscribe"},
        {"params", {
            {"channels", connections}
        }}
    };
    isDataStreaming = true;
    if (!m_active_connections.empty()) {
        int connectionId = m_active_connections.begin()->first;
        send(connectionId, subscribe.dump());
        struct termios oldt, newt;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        int oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
        fmt::print(fmt::fg(fmt::color::blue) | fmt::emphasis::bold,
                "> Streaming... Press 'q' to quit.\n");
        while(isDataStreaming) {
            char ch;
            if (read(STDIN_FILENO, &ch, 1) > 0) {
                if (ch == 'q' || ch == 'Q') {
                    isDataStreaming = false;
                    json unsubscribe = {
                        {"jsonrpc", "2.0"},
                        {"id", 154},
                        {"method", "private/unsubscribe_all"},
                        {"params", {}}
                    };
                    send(connectionId, unsubscribe.dump());
                    break;
                }
            }
            this_thread::sleep_for(chrono::milliseconds(1));
        }
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        fcntl(STDIN_FILENO, F_SETFL, oldf);
        fmt::print(fmt::fg(fmt::color::cyan) | fmt::emphasis::bold,
                "> Streaming stopped.\n");
    } else {
        fmt::print(fmt::fg(fmt::color::red) | fmt::emphasis::bold,
                "> No active connections to send subscribe message.\n");
        return -1;
    }
    return 0;
}