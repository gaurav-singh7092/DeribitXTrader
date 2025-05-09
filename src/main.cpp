#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>
#include <fmt/color.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "network/socket_client.h"
#include "exchange_interface/market_api.h"
#include "helpers/utility.h"
#include "performance/monitor.h"
namespace {
    constexpr int WS_CLOSE_NORMAL = 1000;
}
using namespace std;
int main() {
    bool done = false;
    char* input;
    SocketEndpoint endpoint;
    utils::printHeader();
    while (!done) {
        input = readline(fmt::format(fg(fmt::color::blue), "tradexderibit> ").c_str());
        if (!input) {
            break;
        }
        string command(input);
        free(input);
        if (command.empty()) {
            continue;
        }
        add_history(command.c_str());
        if (command == "quit" || command == "exit") {
            done = true;
        }
        else if (command == "help" || command == "main") {
            utils::printHelp();
        }
        else if (command.substr(0, 7) == "connect") {
            if (command.length() <= 8) {
                fmt::print(fg(fmt::color::red) | fmt::emphasis::bold,
                           "Error: Missing URI. Usage: connect <URI>\n");
            } else {
                string uri = command.substr(8);
                int id = endpoint.connect(uri);
                if (id != -1) {
                    fmt::print(fg(fmt::color::green) | fmt::emphasis::bold,
                               "> Successfully created connection.\n");
                    fmt::print(fg(fmt::color::cyan), "> Connection ID: {}\n", id);
                    fmt::print(fg(fmt::color::yellow), "> Status: {}\n", endpoint.get_metadata(id)->get_status());
                    fmt::print(fmt::fg(fmt::color::white), "> use \"show {}\" to check Status \n", id);
                } else {
                    fmt::print(fg(fmt::color::red) | fmt::emphasis::bold,
                               "Error: Failed to create connection to {}\n", uri);
                }
            }
        }
        else if (command.substr(0, 13) == "show_messages") {
            stringstream ss(command);
            string cmd;
            int id;
            ss >> cmd >> id;
            if (ss.fail()) {
                fmt::print(fg(fmt::color::red) | fmt::emphasis::bold,
                           "Error: Missing connection ID. Usage: show_messages <connection_id>\n");
            } else {
                ConnectionDetails::ptr metadata = endpoint.get_metadata(id);
                if (metadata) {
                    if (metadata->m_received_data.empty()) {
                        fmt::print(fg(fmt::color::yellow), "> No messages for connection {}\n", id);
                    } else {
                        for (const auto& msg : metadata->m_received_data) {
                            cout << msg << "\n\n";
                        }
                    }
                } else {
                    fmt::print(fg(fmt::color::red) | fmt::emphasis::bold,
                               "> Unknown connection id {}\n", id);
                }
            }
        }
        else if (command.substr(0, 19) == "show_latency_report") {
            cout << getPerformanceMonitor().generate_report() << endl;
        }
        else if (command.substr(0, 12) == "reset_report") {
            getPerformanceMonitor().reset();
        }
        else if (command.substr(0, 4) == "show") {
            int id = atoi(command.substr(5).c_str());
            ConnectionDetails::ptr metadata = endpoint.get_metadata(id);
            if (metadata) {
                fmt::print(fg(fmt::color::cyan) | fmt::emphasis::bold, "\n=== Connection Details ===\n");
                fmt::print(fg(fmt::color::green), "Connection ID: {}\n", metadata->get_id());
                fmt::print(fg(fmt::color::yellow), "Status: {}\n", metadata->get_status());
                fmt::print(fg(fmt::color::white), "URI: {}\n", metadata->get_uri());
                fmt::print(fg(fmt::color::white), "Server: {}\n", metadata->get_server());
                fmt::print(fg(fmt::color::magenta), "Messages Count: {}\n", metadata->m_received_data.size());
                if (!metadata->get_error_reason().empty()) {
                    fmt::print(fg(fmt::color::red), "Error: {}\n", metadata->get_error_reason());
                }
                fmt::print("\n");
            } else {
                fmt::print(fg(fmt::color::red) | fmt::emphasis::bold,
                           "Unknown connection id {}\n", id);
            }
        }
        else if (command.substr(0, 5) == "close") {
            stringstream ss(command);
            string cmd;
            int id;
            int close_code = WS_CLOSE_NORMAL;
            string reason;
            ss >> cmd >> id >> close_code;
            getline(ss, reason);
            endpoint.close(id, close_code, reason);
        }
        else if (command.substr(0, 4) == "send") {
            stringstream ss(command);
            string cmd;
            int id;
            string message = "";
            ss >> cmd >> id;
            getline(ss, message);
            endpoint.send(id, message);
            unique_lock<mutex> lock(endpoint.get_metadata(id)->connection_mutex);
            endpoint.get_metadata(id)->connection_cv.wait(lock, [&] { return endpoint.get_metadata(id)->DATA_PROCESSED; });
            endpoint.get_metadata(id)->DATA_PROCESSED = false;
        }
        else if (command == "deribit connect" || command == "Deribit connect") {
            const string uri = "wss://test.deribit.com/ws/api/v2";
            int id = endpoint.connect(uri);
            if (id != -1) {
                fmt::print(fg(fmt::color::green) | fmt::emphasis::bold,
                           "> Successfully created connection to Deribit TESTNET.\n");
                fmt::print(fg(fmt::color::cyan), "> Connection ID: {}\n", id);
                fmt::print(fg(fmt::color::yellow), "> Status: {}\n", endpoint.get_metadata(id)->get_status());
                fmt::print(fmt::fg(fmt::color::white), "> use \"show {}\" to check Status \n", id);
            } else {
                fmt::print(fg(fmt::color::red) | fmt::emphasis::bold,
                           "> Failed to create connection to Deribit TESTNET.\n");
            }
        }
        else if(command == "view_stream"){
            vector<string> connections = api::getActiveSubscription();
            if(connections.size()){
                endpoint.streamSubscriptions(connections);
            } else {
                fmt::print(fg(fmt::color::red) | fmt::emphasis::bold,
                           "> No Subscriptions. Use 'Deribit <id> subscribe <symbol>' to add a subscription.\n");
            }
        }
        else if(command == "view_subscriptions"){
            vector<string> connections = api::getActiveSubscription();
            if(!connections.empty()){
                fmt::print(fg(fmt::color::cyan) | fmt::emphasis::bold,
                           "\n=== Active Market Subscriptions ===\n\n");
                int count = 1;
                for(const auto& connection : connections){
                    size_t prefix_pos = connection.find("deribit_price_index.");
                    if(prefix_pos != string::npos){
                        string index_name = connection.substr(prefix_pos + strlen("deribit_price_index."));
                        fmt::print(fg(fmt::color::green) | fmt::emphasis::bold,
                           "[{}] ", count++);
                        fmt::print(fg(fmt::color::white),
                           "{}\n", index_name);
                    }
                }
                fmt::print("\n");
            } else {
                fmt::print(fg(fmt::color::yellow) | fmt::emphasis::bold,
                           "\n=== No Active Subscriptions ===\n");
                fmt::print(fg(fmt::color::white),
                           "Use 'Deribit <id> subscribe <symbol>' to add a subscription.\n\n");
            }
        }
        else if (command.substr(0, 7) == "deribit" || command.substr(0,7) == "Deribit") {
            int id;
            string cmd;
            stringstream ss(command);
            ss >> cmd >> id;
            string msg = api::processRequest(command);
            if (msg != "") {
                int success = endpoint.send(id, msg);
                if (success >= 0) {
                    unique_lock<mutex> lock(endpoint.get_metadata(id)->connection_mutex);
                    bool result = endpoint.get_metadata(id)->connection_cv.wait_for(
                        lock,
                        chrono::seconds(10),
                        [&] { return endpoint.get_metadata(id)->DATA_PROCESSED; }
                    );
                    if (!result) {
                        fmt::print(fg(fmt::color::red) | fmt::emphasis::bold,
                                  "> Request timed out. The server did not respond in time.\n");
                    }
                    endpoint.get_metadata(id)->DATA_PROCESSED = false;
                } else {
                    fmt::print(fg(fmt::color::red) | fmt::emphasis::bold,
                              "> Failed to send request to the server. Check your connection.\n");
                }
            } else {
                fmt::print(fg(fmt::color::yellow) | fmt::emphasis::bold,
                          "> Request preparation failed. Please check your input parameters.\n");
            }
        }
        else {
            fmt::print(fg(fmt::color::yellow), "> Unrecognized command\n");
        }
    }
    return 0;
}
