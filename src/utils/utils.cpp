#include <iostream>
#include <sstream>
#include <iomanip>
#include "utils/utils.h"
#include <chrono>
#include <time.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include "json/json.hpp"
#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif
#include <fcntl.h>
using namespace std;
using json = nlohmann::json;
int utils::getTerminalWidth() {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        return w.ws_col;
    }
    return 80; 
}
void utils::printHeader() {
    int terminal_width = utils::getTerminalWidth();
    string title = "🚀 DERIBitXTrader";
    string subtitle = "💹 Crypto Trading Platform";
    string border(terminal_width, '=');
    string thin_border(terminal_width, '-');
    fmt::print(fg(fmt::rgb(230, 230, 230)) | bg(fmt::rgb(70, 90, 110)) | fmt::emphasis::bold,
               "{:^{}}\n", title, terminal_width);
    fmt::print(fg(fmt::rgb(100, 140, 140)) | fmt::emphasis::italic,
               "{:^{}}\n", subtitle, terminal_width);
    fmt::print(fg(fmt::rgb(120, 120, 120)) | fmt::emphasis::bold, "{}\n", border);
    fmt::print("\n");
}
void utils::printHelp() {
    int terminal_width = utils::getTerminalWidth();
    string separator(terminal_width, '=');
    string thin_separator(terminal_width, '-');
    fmt::print(fg(fmt::rgb(204, 153, 0)) | fmt::emphasis::bold, "\n{}\n", separator);
    fmt::print(fg(fmt::color::white) | bg(fmt::rgb(51, 102, 153)) | fmt::emphasis::bold,
               "{:^{}}\n", "📚 COMMAND REFERENCE", terminal_width);
    fmt::print(fg(fmt::rgb(204, 153, 0)) | fmt::emphasis::bold, "{}\n\n", separator);
    fmt::print(fg(fmt::rgb(100, 130, 100)) | fmt::emphasis::bold, "🔧 GENERAL COMMANDS:\n");
    fmt::print(fg(fmt::rgb(130, 130, 130)), "{}\n", thin_separator);
    fmt::print(fg(fmt::rgb(220, 220, 220)) | fmt::emphasis::bold, "  {:<30} : ", "> help");
    fmt::print(fg(fmt::rgb(79, 134, 140)), "{}\n", "❓ Displays this help text");
    fmt::print(fg(fmt::rgb(220, 220, 220)) | fmt::emphasis::bold, "  {:<30} : ", "> quit / exit");
    fmt::print(fg(fmt::rgb(79, 134, 140)), "{}\n", "🚪 Exits the program");
    fmt::print(fg(fmt::rgb(220, 220, 220)) | fmt::emphasis::bold, "  {:<30} : ", "> connect <URI>");
    fmt::print(fg(fmt::rgb(79, 134, 140)), "{}\n", "🔌 Creates a WebSocket connection with the given URI");
    fmt::print(fg(fmt::rgb(220, 220, 220)) | fmt::emphasis::bold, "  {:<30} : ", "> close <id> [code] [reason]");
    fmt::print(fg(fmt::rgb(79, 134, 140)), "{}\n", "🔒 Closes the WebSocket connection with the specified ID");
    fmt::print(fg(fmt::rgb(220, 220, 220)) | fmt::emphasis::bold, "  {:<30} : ", "> show <id>");
    fmt::print(fg(fmt::rgb(79, 134, 140)), "{}\n", "🔍 Displays metadata for the specified connection");
    fmt::print(fg(fmt::rgb(220, 220, 220)) | fmt::emphasis::bold, "  {:<30} : ", "> show_messages <id>");
    fmt::print(fg(fmt::rgb(79, 134, 140)), "{}\n", "📋 Lists all messages sent and received on the specified connection");
    fmt::print(fg(fmt::rgb(220, 220, 220)) | fmt::emphasis::bold, "  {:<30} : ", "> send <id> <message>");
    fmt::print(fg(fmt::rgb(79, 134, 140)), "{}\n", "📤 Sends a message to the specified connection");
    fmt::print(fg(fmt::rgb(220, 220, 220)) | fmt::emphasis::bold, "  {:<30} : ", "> view_subscriptions");
    fmt::print(fg(fmt::rgb(79, 134, 140)), "{}\n", "📊 Displays the list of subscribed symbols for orderbook updates");
    fmt::print(fg(fmt::rgb(220, 220, 220)) | fmt::emphasis::bold, "  {:<30} : ", "> view_stream");
    fmt::print(fg(fmt::rgb(79, 134, 140)), "{}\n", "📈 Displays the stream of orderbook updates for subscribed symbols");
    fmt::print(fg(fmt::rgb(220, 220, 220)) | fmt::emphasis::bold, "  {:<30} : ", "> show_latency_report");
    fmt::print(fg(fmt::rgb(79, 134, 140)), "{}\n", "⏱️ Generates a performance latency report for the current session");
    fmt::print(fg(fmt::rgb(220, 220, 220)) | fmt::emphasis::bold, "  {:<30} : ", "> reset_report");
    fmt::print(fg(fmt::rgb(79, 134, 140)), "{}\n\n", "🔄 Clears the latency report data for the current session");
    fmt::print(fg(fmt::rgb(153, 120, 89)) | fmt::emphasis::bold, "💹 DERIBIT API COMMANDS:\n");
    fmt::print(fg(fmt::rgb(130, 130, 130)), "{}\n", thin_separator);
    fmt::print(fg(fmt::rgb(153, 133, 89)) | fmt::emphasis::bold, "  🔐 Connection and Authentication:\n");
    fmt::print(fg(fmt::rgb(220, 220, 220)) | fmt::emphasis::bold, "  {:<60} : ", "> deribit connect");
    fmt::print(fg(fmt::rgb(79, 134, 140)), "{}\n", "🌐 Establish a new WebSocket connection to Deribit's testnet");
    fmt::print(fg(fmt::rgb(220, 220, 220)) | fmt::emphasis::bold, "  {:<60} : ", "> Deribit <id> authorize <client_id> <client_secret> [-s]");
    fmt::print(fg(fmt::rgb(79, 134, 140)), "{}\n\n", "🔑 Authenticate and retrieve an access token; use -s to persist token in session");
    fmt::print(fg(fmt::rgb(153, 133, 89)) | fmt::emphasis::bold, "  📝 Order Management:\n");
    fmt::print(fg(fmt::rgb(220, 220, 220)) | fmt::emphasis::bold, "  {:<60} : ", "> Deribit <id> buy <instrument> [comments]");
    fmt::print(fg(fmt::rgb(79, 134, 140)), "{}\n", "🟢 Place a buy market or limit order for the specified instrument");
    fmt::print(fg(fmt::rgb(220, 220, 220)) | fmt::emphasis::bold, "  {:<60} : ", "> Deribit <id> sell <instrument> [comments]");
    fmt::print(fg(fmt::rgb(79, 134, 140)), "{}\n", "🔴 Place a sell market or limit order for the specified instrument");
    fmt::print(fg(fmt::rgb(220, 220, 220)) | fmt::emphasis::bold, "  {:<60} : ", "> Deribit <id> modify <order_id>");
    fmt::print(fg(fmt::rgb(79, 134, 140)), "{}\n", "✏️ Update price or quantity of an active order");
    fmt::print(fg(fmt::rgb(220, 220, 220)) | fmt::emphasis::bold, "  {:<60} : ", "> Deribit <id> cancel <order_id>");
    fmt::print(fg(fmt::rgb(79, 134, 140)), "{}\n", "❌ Cancel a specific order by its order ID");
    fmt::print(fg(fmt::rgb(220, 220, 220)) | fmt::emphasis::bold, "  {:<60} : ", "> Deribit <id> cancel_all");
    fmt::print(fg(fmt::rgb(79, 134, 140)), "{}\n\n", "🧹 Cancel all active orders for the current account");
    fmt::print(fg(fmt::rgb(153, 133, 89)) | fmt::emphasis::bold, "  📊 Information Retrieval:\n");
    fmt::print(fg(fmt::rgb(220, 220, 220)) | fmt::emphasis::bold, "  {:<60} : ", "> Deribit <id> get_open_orders {options}");
    fmt::print(fg(fmt::rgb(79, 134, 140)), "{}\n", "📋 Retrieve open orders with optional filtering");
    fmt::print(fg(fmt::rgb(220, 220, 220)) | fmt::emphasis::bold, "  {:<60} : ", "> Deribit <id> positions [currency] [kind]");
    fmt::print(fg(fmt::rgb(79, 134, 140)), "{}\n", "💼 Fetch current open positions, optionally filtered by currency or instrument type");
    fmt::print(fg(fmt::rgb(220, 220, 220)) | fmt::emphasis::bold, "  {:<60} : ", "> Deribit <id> orderbook <instrument> [depth]");
    fmt::print(fg(fmt::rgb(79, 134, 140)), "{}\n\n", "📈 View current buy and sell orders for an instrument, with optional depth limit");
    fmt::print(fg(fmt::rgb(153, 133, 89)) | fmt::emphasis::bold, "  📡 Symbol Subscription:\n");
    fmt::print(fg(fmt::rgb(220, 220, 220)) | fmt::emphasis::bold, "  {:<60} : ", "> Deribit <id> subscribe [symbol]");
    fmt::print(fg(fmt::rgb(79, 134, 140)), "{}\n", "📥 Subscribes to that symbol to stream continuous orderbook updates");
    fmt::print(fg(fmt::rgb(220, 220, 220)) | fmt::emphasis::bold, "  {:<60} : ", "> Deribit <id> unsubscribe [symbol]");
    fmt::print(fg(fmt::rgb(79, 134, 140)), "{}\n", "📤 Unsubscribes to that symbol stream continuous orderbook updates");
    fmt::print(fg(fmt::rgb(220, 220, 220)) | fmt::emphasis::bold, "  {:<60} : ", "> Deribit <id> unsubscribe_all");
    fmt::print(fg(fmt::rgb(79, 134, 140)), "{}\n", "🚫 Unsubscribes to all symbols that have been subscribed to stream real time data");
    fmt::print(fg(fmt::rgb(204, 153, 0)) | fmt::emphasis::bold, "\n{}\n\n", separator);
}
void utils::printcmd(string const &str){
    fmt::print(fg(fmt::rgb(150, 150, 170)), str);
}
void utils::printcmd(string const &str, int r, int g, int b){
    fmt::print(fg(fmt::rgb(r, g, b)), str);
}
void utils::printerr(string const &str){
    fmt::print(fg(fmt::rgb(140, 80, 80)) | fmt::emphasis::bold, "❌ {}", str);
}
void utils::printsuccess(string const &str){
    fmt::print(fg(fmt::rgb(100, 130, 100)) | fmt::emphasis::bold, "✅ {}\n", str);
}
void utils::printinfo(string const &str){
    fmt::print(fg(fmt::rgb(100, 130, 160)), "ℹ️  {}\n", str);
}
void utils::printwarning(string const &str){
    fmt::print(fg(fmt::rgb(153, 133, 89)) | fmt::emphasis::bold, "⚠️  {}\n", str);
}
long long utils::getCurrentTimestamp(){
    auto now = chrono::system_clock::now();
    auto now_ms = chrono::time_point_cast<chrono::milliseconds>(now);
    auto epoch = now_ms.time_since_epoch();
    long long unix_timestamp_ms = epoch.count();
    return unix_timestamp_ms;
}
string utils::generateRandomString(const int len) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    string tmp_s;
    tmp_s.reserve(len);
    for (int i = 0; i < len; ++i) {
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    return tmp_s;
}
string utils::convertToHexString(const unsigned char* data, unsigned int length) {
    ostringstream hex_stream;
    hex_stream << hex << uppercase << setfill('0');
    for (unsigned int i = 0; i < length; ++i) {
        hex_stream << setw(2) << static_cast<int>(data[i]);
    }
    return hex_stream.str();
}
string utils::generateHmacSha256(const string& key, const string& data) {
    unsigned char result[EVP_MAX_MD_SIZE];
    unsigned int result_length = 0;
    HMAC(EVP_sha256(), key.c_str(), key.length(),
         reinterpret_cast<const unsigned char*>(data.c_str()), data.length(),
         result, &result_length);
    return utils::convertToHexString(result, result_length);
}
string utils::createSignature(long long timestamp, string nonce, string data, string clientsecret){
    string string_to_code = to_string(timestamp) + "\n" + nonce + "\n" + data;
    return utils::generateHmacSha256(clientsecret, string_to_code);
}
string utils::formatJson(string j) {
    json serialised = json::parse(j);
    return serialised.dump(4);
}
string utils::mapToString(map<string, string> mpp) {
    ostringstream os;
    for (const auto& pair : mpp) {
        os << pair.first << " : " << pair.second << '\n'; 
    }
    return os.str();
}
string utils::securePasswordInput() {
    string password;
    char ch;
    #ifdef _WIN32
        while ((ch = _getch()) != '\r') {
            if (ch == '\b') {
                if (!password.empty()) {
                    password.pop_back();
                    cout << "\b \b";
                }
            } else {
                password += ch;
                cout << '*';
            }
        }
        cout << endl;
    #else
        struct termios oldt, newt;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~ECHO;
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        cin >> password;
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        cout << endl;
    #endif
    return password;
}
void utils::clear_console() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}
bool utils::is_key_pressed(char key) {
    struct termios oldt, newt;
    int ch;
    int oldf;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    if (ch == key) {
        while (getchar() != EOF);
        return true;
    }
    return false;
}
void utils::printOrderbook(const string &instrument, const string &data, int depth) {
    int terminal_width = utils::getTerminalWidth();
    string separator(terminal_width, '-');
    json orderbook;
    try {
        orderbook = json::parse(data);
    } catch (json::parse_error& e) {
        printerr("❌ Error parsing orderbook data: " + string(e.what()) + "\n");
        return;
    }
    fmt::print(fg(fmt::rgb(240, 240, 240)) | bg(fmt::rgb(51, 102, 153)) | fmt::emphasis::bold, "\n{:^{}}\n",
               "📊 ORDERBOOK: " + instrument, terminal_width);
    fmt::print(fg(fmt::rgb(150, 150, 150)), "{}\n", separator);
    string timestamp = "";
    if (orderbook.contains("timestamp")) {
        if (orderbook["timestamp"].is_string()) {
            timestamp = orderbook["timestamp"].get<string>();
        } else if (orderbook["timestamp"].is_number()) {
            timestamp = to_string(orderbook["timestamp"].get<int64_t>());
        }
        if (!timestamp.empty()) {
            auto ts = stoull(timestamp);
            auto tp = chrono::time_point<chrono::system_clock>(chrono::milliseconds(ts));
            auto time = chrono::system_clock::to_time_t(tp);
            timestamp = string(ctime(&time));
            timestamp.pop_back(); 
        }
    }
    fmt::print(fg(fmt::rgb(150, 150, 150)), "⏰ Time: ");
    fmt::print(fg(fmt::rgb(204, 173, 0)), "{}\n\n", timestamp);
    int column_width = terminal_width / 4;
    fmt::print(fg(fmt::rgb(240, 240, 240)) | fmt::emphasis::bold,
               "{:^{}} | {:^{}} | {:^{}} | {:^{}}\n",
               "💰 BID AMOUNT", column_width,
               "📊 BID PRICE", column_width,
               "📈 ASK PRICE", column_width,
               "💵 ASK AMOUNT", column_width);
    fmt::print(fg(fmt::rgb(150, 150, 150)), "{}\n", separator);
    vector<vector<string>> bids;
    vector<vector<string>> asks;
    if (orderbook.contains("result") &&
        orderbook["result"].contains("bids") &&
        orderbook["result"].contains("asks")) {
        for (const auto& bid : orderbook["result"]["bids"]) {
            if (bids.size() >= depth) break;
            string price;
            if (bid[0].is_string()) {
                price = bid[0].get<string>();
            } else if (bid[0].is_number()) {
                price = to_string(bid[0].get<double>());
            } else {
                price = "N/A";
            }
            string amount;
            if (bid[1].is_string()) {
                amount = bid[1].get<string>();
            } else if (bid[1].is_number()) {
                amount = to_string(bid[1].get<double>());
            } else {
                amount = "N/A";
            }
            bids.push_back({price, amount});
        }
        for (const auto& ask : orderbook["result"]["asks"]) {
            if (asks.size() >= depth) break;
            string price;
            if (ask[0].is_string()) {
                price = ask[0].get<string>();
            } else if (ask[0].is_number()) {
                price = to_string(ask[0].get<double>());
            } else {
                price = "N/A";
            }
            string amount;
            if (ask[1].is_string()) {
                amount = ask[1].get<string>();
            } else if (ask[1].is_number()) {
                amount = to_string(ask[1].get<double>());
            } else {
                amount = "N/A";
            }
            asks.push_back({price, amount});
        }
        size_t max_rows = max(bids.size(), asks.size());
        for (size_t i = 0; i < max_rows; i++) {
            if (i < bids.size()) {
                fmt::print(fg(fmt::rgb(102, 153, 204)), "{:^{}}", bids[i][1], column_width);
                fmt::print(" | ");
                fmt::print(fg(fmt::rgb(102, 153, 102)) | fmt::emphasis::bold, "{:^{}}", bids[i][0], column_width);
            } else {
                fmt::print("{:^{}}", "", column_width);
                fmt::print(" | ");
                fmt::print("{:^{}}", "", column_width);
            }
            if (i < asks.size()) {
                fmt::print(" | ");
                fmt::print(fg(fmt::rgb(153, 51, 51)) | fmt::emphasis::bold, "{:^{}}", asks[i][0], column_width);
                fmt::print(" | ");
                fmt::print(fg(fmt::rgb(102, 153, 204)), "{:^{}}", asks[i][1], column_width);
            } else {
                fmt::print(" | ");
                fmt::print("{:^{}}", "", column_width);
                fmt::print(" | ");
                fmt::print("{:^{}}", "", column_width);
            }
            fmt::print("\n");
        }
    } else {
        printerr("❌ Invalid orderbook format or empty orderbook\n");
    }
    fmt::print(fg(fmt::rgb(180, 180, 180)), "{}\n", separator);
}
void utils::printPositions(const string &data) {
    int terminal_width = utils::getTerminalWidth();
    string separator(terminal_width, '-');
    json positions_data;
    try {
        positions_data = json::parse(data);
    } catch (json::parse_error& e) {
        printerr("❌ Error parsing positions data: " + string(e.what()) + "\n");
        return;
    }
    fmt::print(fg(fmt::rgb(240, 240, 240)) | bg(fmt::rgb(51, 102, 153)) | fmt::emphasis::bold, "\n{:^{}}\n",
               "📊 POSITIONS SUMMARY", terminal_width);
    fmt::print(fg(fmt::rgb(150, 150, 150)), "{}\n", separator);
    if (positions_data.contains("result") && positions_data["result"].is_array()) {
        auto& positions = positions_data["result"];
        if (positions.empty()) {
            fmt::print(fg(fmt::rgb(204, 173, 0)) | fmt::emphasis::italic,
                      "{:^{}}\n", "💼 No open positions", terminal_width);
        } else {
            fmt::print(fg(fmt::rgb(240, 240, 240)) | fmt::emphasis::bold,
                      "{:<25} {:<10} {:<12} {:<15} {:<15} {:<15}\n",
                      "🎯 INSTRUMENT", "📈 DIRECTION", "📊 SIZE", "💰 ENTRY", "📉 MARK", "💵 PNL");
            fmt::print(fg(fmt::rgb(150, 150, 150)), "{}\n", separator);
            for (const auto& pos : positions) {
                string instrument = pos["instrument_name"];
                string direction = pos["direction"];
                string size;
                if (pos["size"].is_string()) {
                    size = pos["size"].get<string>();
                } else if (pos["size"].is_number()) {
                    size = to_string(pos["size"].get<double>());
                } else {
                    size = "N/A";
                }
                string entry_price;
                if (pos["average_price"].is_string()) {
                    entry_price = pos["average_price"].get<string>();
                } else if (pos["average_price"].is_number()) {
                    entry_price = to_string(pos["average_price"].get<double>());
                } else {
                    entry_price = "N/A";
                }
                string mark_price;
                if (pos["mark_price"].is_string()) {
                    mark_price = pos["mark_price"].get<string>();
                } else if (pos["mark_price"].is_number()) {
                    mark_price = to_string(pos["mark_price"].get<double>());
                } else {
                    mark_price = "N/A";
                }
                string pnl;
                if (pos["floating_profit_loss"].is_string()) {
                    pnl = pos["floating_profit_loss"].get<string>();
                } else if (pos["floating_profit_loss"].is_number()) {
                    pnl = to_string(pos["floating_profit_loss"].get<double>());
                } else {
                    pnl = "N/A";
                }
                auto dir_color = (direction == "buy") ? fmt::rgb(102, 153, 102) : fmt::rgb(153, 51, 51);
                auto pnl_color = (stod(pnl) >= 0) ? fmt::rgb(102, 153, 102) : fmt::rgb(153, 51, 51);
                fmt::print(fg(fmt::rgb(240, 240, 240)), "{:<25} ", instrument);
                fmt::print(fg(dir_color) | fmt::emphasis::bold, "{:<10} ", direction);
                fmt::print(fg(fmt::rgb(102, 153, 204)), "{:<12} ", size);
                fmt::print(fg(fmt::rgb(102, 153, 204)), "{:<15} ", entry_price);
                fmt::print(fg(fmt::rgb(102, 153, 204)), "{:<15} ", mark_price);
                fmt::print(fg(pnl_color) | fmt::emphasis::bold, "{:<15}\n", pnl);
            }
        }
    } else {
        printerr("❌ Invalid positions data format or positions data not found\n");
    }
    fmt::print(fg(fmt::rgb(180, 180, 180)), "{}\n", separator);
}
void utils::printOpenOrders(const string &data) {
    int terminal_width = utils::getTerminalWidth();
    string separator(terminal_width, '-');
    json orders_data;
    try {
        orders_data = json::parse(data);
    } catch (json::parse_error& e) {
        printerr("Error parsing orders data: " + string(e.what()) + "\n");
        return;
    }
    fmt::print(fg(fmt::rgb(240, 240, 240)) | bg(fmt::rgb(51, 102, 153)) | fmt::emphasis::bold, "\n{:^{}}\n",
               "OPEN ORDERS", terminal_width);
    fmt::print(fg(fmt::rgb(150, 150, 150)), "{}\n", separator);
    if (orders_data.contains("result") && orders_data["result"].is_array()) {
        auto& orders = orders_data["result"];
        if (orders.empty()) {
            fmt::print(fg(fmt::rgb(204, 173, 0)) | fmt::emphasis::italic,
                      "{:^{}}\n", "No open orders", terminal_width);
        } else {
            fmt::print(fg(fmt::rgb(240, 240, 240)) | fmt::emphasis::bold,
                      "{:<12} {:<25} {:<10} {:<12} {:<12} {:<12} {:<15}\n",
                      "ORDER ID", "INSTRUMENT", "DIRECTION", "AMOUNT", "FILLED", "PRICE", "ORDER TYPE");
            fmt::print(fg(fmt::rgb(150, 150, 150)), "{}\n", separator);
            for (const auto& order : orders) {
                string order_id = order["order_id"];
                string instrument = order["instrument_name"];
                string direction = order["direction"];
                string amount, filled_amount, price;
                if (order["amount"].is_string()) {
                    amount = order["amount"].get<string>();
                } else if (order["amount"].is_number()) {
                    amount = to_string(order["amount"].get<double>());
                } else {
                    amount = "N/A";
                }
                if (order["filled_amount"].is_string()) {
                    filled_amount = order["filled_amount"].get<string>();
                } else if (order["filled_amount"].is_number()) {
                    filled_amount = to_string(order["filled_amount"].get<double>());
                } else {
                    filled_amount = "N/A";
                }
                if (order["price"].is_string()) {
                    price = order["price"].get<string>();
                } else if (order["price"].is_number()) {
                    price = to_string(order["price"].get<double>());
                } else {
                    price = "N/A";
                }
                string order_type = order["order_type"];
                if (order_id.length() > 10) {
                    order_id = order_id.substr(0, 8) + "..";
                }
                auto dir_color = (direction == "buy") ? fmt::rgb(102, 153, 102) : fmt::rgb(153, 51, 51);
                fmt::print(fg(fmt::rgb(102, 153, 204)), "{:<12} ", order_id);
                fmt::print(fg(fmt::rgb(240, 240, 240)), "{:<25} ", instrument);
                fmt::print(fg(dir_color) | fmt::emphasis::bold, "{:<10} ", direction);
                fmt::print(fg(fmt::rgb(102, 153, 204)), "{:<12} ", amount);
                fmt::print(fg(fmt::rgb(102, 153, 204)), "{:<12} ", filled_amount);
                fmt::print(fg(fmt::rgb(102, 153, 204)), "{:<12} ", price);
                fmt::print(fg(fmt::rgb(204, 173, 0)), "{:<15}\n", order_type);
            }
        }
    } else {
        printerr("Invalid orders data format or orders data not found\n");
    }
    fmt::print(fg(fmt::rgb(180, 180, 180)), "{}\n", separator);
}
void utils::printTradeConfirmation(const string &data) {
    int terminal_width = utils::getTerminalWidth();
    string separator(terminal_width, '-');
    json trade_data;
    try {
        trade_data = json::parse(data);
    } catch (json::parse_error& e) {
        printerr("Error parsing trade confirmation data: " + string(e.what()) + "\n");
        return;
    }
    bool success = false;
    if (trade_data.contains("result") && !trade_data["result"].is_null()) {
        success = true;
    }
    if (success) {
        fmt::print(fg(fmt::rgb(100, 130, 100)) | fmt::emphasis::bold, "\n{:^{}}\n",
                   "✓ TRADE EXECUTED SUCCESSFULLY", terminal_width);
    } else {
        fmt::print(fg(fmt::rgb(140, 80, 80)) | fmt::emphasis::bold, "\n{:^{}}\n",
                   "✗ TRADE EXECUTION FAILED", terminal_width);
    }
    fmt::print(fg(fmt::rgb(130, 130, 130)), "{}\n", separator);
    if (success && trade_data.contains("result")) {
        auto& result = trade_data["result"];
        string order_id = result["order"]["order_id"];
        string instrument = result["order"]["instrument_name"];
        string direction = result["order"]["direction"];
        string price = result["order"]["price"].get<string>();
        string amount = result["order"]["amount"].get<string>();
        string order_type = result["order"]["order_type"];
        fmt::print(fg(fmt::rgb(220, 220, 220)) | fmt::emphasis::bold, "ORDER DETAILS:\n\n");
        fmt::print(fg(fmt::rgb(220, 220, 220)), "{:<15} : ", "Order ID");
        fmt::print(fg(fmt::rgb(79, 134, 140)), "{}\n", order_id);
        fmt::print(fg(fmt::rgb(220, 220, 220)), "{:<15} : ", "Instrument");
        fmt::print(fg(fmt::rgb(79, 134, 140)), "{}\n", instrument);
        fmt::print(fg(fmt::rgb(220, 220, 220)), "{:<15} : ", "Direction");
        if (direction == "buy") {
            fmt::print(fg(fmt::rgb(100, 130, 100)) | fmt::emphasis::bold, "{}\n", "BUY");
        } else {
            fmt::print(fg(fmt::rgb(140, 80, 80)) | fmt::emphasis::bold, "{}\n", "SELL");
        }
        fmt::print(fg(fmt::rgb(220, 220, 220)), "{:<15} : ", "Price");
        fmt::print(fg(fmt::rgb(79, 134, 140)), "{}\n", price);
        fmt::print(fg(fmt::rgb(220, 220, 220)), "{:<15} : ", "Amount");
        fmt::print(fg(fmt::rgb(79, 134, 140)), "{}\n", amount);
        fmt::print(fg(fmt::rgb(220, 220, 220)), "{:<15} : ", "Order Type");
        fmt::print(fg(fmt::rgb(79, 134, 140)), "{}\n", order_type);
        if (result.contains("trades") && result["trades"].is_array() && !result["trades"].empty()) {
            fmt::print(fg(fmt::rgb(220, 220, 220)) | fmt::emphasis::bold, "\nEXECUTED TRADES:\n\n");
            int trade_num = 1;
            for (const auto& trade : result["trades"]) {
                fmt::print(fg(fmt::rgb(153, 133, 89)) | fmt::emphasis::bold, "Trade #{}\n", trade_num++);
                fmt::print(fg(fmt::rgb(220, 220, 220)), "{:<15} : ", "Trade ID");
                fmt::print(fg(fmt::rgb(79, 134, 140)), "{}\n", trade["trade_id"].get<string>());
                fmt::print(fg(fmt::rgb(220, 220, 220)), "{:<15} : ", "Price");
                fmt::print(fg(fmt::rgb(79, 134, 140)), "{}\n", trade["price"].get<string>());
                fmt::print(fg(fmt::rgb(220, 220, 220)), "{:<15} : ", "Amount");
                fmt::print(fg(fmt::rgb(79, 134, 140)), "{}\n", trade["amount"].get<string>());
                fmt::print(fg(fmt::rgb(220, 220, 220)), "{:<15} : ", "Fee");
                fmt::print(fg(fmt::rgb(79, 134, 140)), "{} {}\n",
                           trade["fee"].get<string>(), trade["fee_currency"].get<string>());
                fmt::print("\n");
            }
        }
    } else if (!success && trade_data.contains("error")) {
        fmt::print(fg(fmt::rgb(220, 220, 220)) | fmt::emphasis::bold, "ERROR DETAILS:\n\n");
        fmt::print(fg(fmt::rgb(220, 220, 220)), "{:<15} : ", "Code");
        fmt::print(fg(fmt::rgb(140, 80, 80)), "{}\n", trade_data["error"]["code"].get<int>());
        fmt::print(fg(fmt::rgb(220, 220, 220)), "{:<15} : ", "Message");
        fmt::print(fg(fmt::rgb(140, 80, 80)), "{}\n", trade_data["error"]["message"].get<string>());
    }
    fmt::print(fg(fmt::rgb(130, 130, 130)), "{}\n", separator);
}
void utils::printSubscriptionStatus(const vector<string> &subscriptions) {
    int terminal_width = utils::getTerminalWidth();
    string separator(terminal_width, '-');
    fmt::print(fg(fmt::rgb(220, 220, 220)) | bg(fmt::rgb(75, 101, 132)) | fmt::emphasis::bold, "\n{:^{}}\n",
               "CURRENT SUBSCRIPTIONS", terminal_width);
    fmt::print(fg(fmt::rgb(130, 130, 130)), "{}\n", separator);
    if (subscriptions.empty()) {
        fmt::print(fg(fmt::rgb(153, 133, 89)) | fmt::emphasis::italic,
                  "{:^{}}\n", "No active subscriptions", terminal_width);
    } else {
        fmt::print(fg(fmt::rgb(220, 220, 220)) | fmt::emphasis::bold, "{:<10} {:<50}\n",
                  "#", "INSTRUMENT");
        fmt::print(fg(fmt::rgb(130, 130, 130)), "{}\n", separator);
        int count = 1;
        for (const auto& sub : subscriptions) {
            fmt::print(fg(fmt::rgb(130, 130, 130)), "{:<10} ", count++);
            fmt::print(fg(fmt::rgb(79, 134, 140)), "{:<50}\n", sub);
        }
    }
    fmt::print(fg(fmt::rgb(130, 130, 130)), "{}\n", separator);
}
void utils::printLatencyReport(const map<string, double> &latencyData) {
    int terminal_width = utils::getTerminalWidth();
    string separator(terminal_width, '-');
    fmt::print(fg(fmt::rgb(220, 220, 220)) | bg(fmt::rgb(75, 101, 132)) | fmt::emphasis::bold, "\n{:^{}}\n",
               "LATENCY PERFORMANCE REPORT", terminal_width);
    fmt::print(fg(fmt::rgb(130, 130, 130)), "{}\n", separator);
    if (latencyData.empty()) {
        fmt::print(fg(fmt::rgb(153, 133, 89)) | fmt::emphasis::italic,
                  "{:^{}}\n", "No latency data available", terminal_width);
    } else {
        fmt::print(fg(fmt::rgb(220, 220, 220)) | fmt::emphasis::bold, "{:<40} {:<20}\n",
                  "OPERATION", "AVG TIME (MS)");
        fmt::print(fg(fmt::rgb(130, 130, 130)), "{}\n", separator);
        for (const auto& [operation, time] : latencyData) {
            fmt::print(fg(fmt::rgb(220, 220, 220)), "{:<40} ", operation);
            if (time < 50) {
                fmt::print(fg(fmt::rgb(100, 130, 100)) | fmt::emphasis::bold, "{:<20.2f}\n", time);
            } else if (time < 200) {
                fmt::print(fg(fmt::rgb(153, 133, 89)) | fmt::emphasis::bold, "{:<20.2f}\n", time);
            } else {
                fmt::print(fg(fmt::rgb(140, 80, 80)) | fmt::emphasis::bold, "{:<20.2f}\n", time);
            }
        }
    }
    fmt::print(fg(fmt::rgb(130, 130, 130)), "{}\n", separator);
}
void utils::printStatusBar(const string &status, bool isConnected) {
    int terminal_width = utils::getTerminalWidth();
    string separator(terminal_width, '-');
    auto now = chrono::system_clock::now();
    auto now_time = chrono::system_clock::to_time_t(now);
    string time_str = ctime(&now_time);
    if (!time_str.empty()) {
        time_str.pop_back(); 
    }
    fmt::print(fg(fmt::rgb(130, 130, 130)), "{}\n", separator);
    if (isConnected) {
        fmt::print(fg(fmt::rgb(100, 130, 100)) | fmt::emphasis::bold, "🟢 CONNECTED | ");
    } else {
        fmt::print(fg(fmt::rgb(140, 80, 80)) | fmt::emphasis::bold, "🔴 DISCONNECTED | ");
    }
    fmt::print(fg(fmt::rgb(220, 220, 220)), "📡 {} | ", status);
    fmt::print(fg(fmt::rgb(100, 130, 160)), "🕒 {}\n", time_str);
    fmt::print(fg(fmt::rgb(130, 130, 130)), "{}\n", separator);
}
void utils::drawProgressBar(int percent) {
    int terminal_width = utils::getTerminalWidth();
    int bar_width = terminal_width - 10; 
    int filled_width = bar_width * percent / 100;
    fmt::print(fg(fmt::rgb(220, 220, 220)), "⏳ [");
    for (int i = 0; i < filled_width; ++i) {
        fmt::print(fg(fmt::rgb(100, 130, 100)), "█");
    }
    for (int i = filled_width; i < bar_width; ++i) {
        fmt::print(fg(fmt::rgb(130, 130, 130)), "░");
    }
    fmt::print(fg(fmt::rgb(220, 220, 220)), "] ");
    fmt::print(fg(fmt::rgb(220, 220, 220)) | fmt::emphasis::bold, "{:3d}% {}\r",
               percent, percent == 100 ? "✨" : "");
    cout << flush;
}
bool utils::check_key_pressed(char key) {
    int key_pressed = is_key_pressed(key);
    if (key_pressed) {
        string key_str(1, key);
        fmt::print(fg(fmt::rgb(100, 130, 160)), "\nKey '{}' pressed. Processing...\n", key_str);
        return true;
    }
    return false;
}
void utils::displayBox(const string &title, const vector<pair<string, string>> &content,
                      fmt::rgb boxColor, const string &icon) {
    int terminal_width = utils::getTerminalWidth();
    int content_width = terminal_width - 4; 
    string horizontal_line(terminal_width - 2, '-');
    string top_border = "╭" + horizontal_line + "╮";
    string bottom_border = "╰" + horizontal_line + "╯";
    string middle_border = "├" + horizontal_line + "┤";
    fmt::print(fg(boxColor), "{}\n", top_border);
    string titled = " " + icon + " " + title + " ";
    int padding = (terminal_width - titled.length()) / 2;
    fmt::print(fg(boxColor), "│");
    fmt::print(fg(fmt::rgb(240, 240, 240)) | bg(boxColor) | fmt::emphasis::bold,
               "{:^{}}", titled, terminal_width - 2);
    fmt::print(fg(boxColor), "│\n");
    fmt::print(fg(boxColor), "{}\n", middle_border);
    for (const auto& [key, value] : content) {
        if (key.empty() && value.empty()) {
            fmt::print(fg(boxColor), "│{:^{}}│\n", "", terminal_width - 2);
        } else {
            fmt::print(fg(boxColor), "│ ");
            fmt::print(fg(fmt::rgb(220, 220, 220)) | fmt::emphasis::bold, "{:<20}", key);
            fmt::print(fg(fmt::rgb(79, 134, 140)), "{:<{}}", value, content_width - 20 - 1);
            fmt::print(fg(boxColor), " │\n");
        }
    }
    fmt::print(fg(boxColor), "{}\n", bottom_border);
}
