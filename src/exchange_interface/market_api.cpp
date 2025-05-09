#include "exchange_interface/market_api.h"
#include "helpers/utility.h"
#include "data_format/json_parser.hpp"
#include "security/credentials.h"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <functional>
#include <regex>
#include <set>
#include <fmt/color.h>
#include "performance/monitor.h"
using namespace std;
using json = nlohmann::json;
bool AUTHENTICATION_SENT = false;
vector<string> AVAILABLE_CURRENCIES = {"BTC", "ETH", "SOL", "XRP", "MATIC",
                                        "USDC", "USDT", "JPY", "CAD", "AUD", "GBP",
                                        "EUR", "USD", "CHF", "BRL", "MXN", "COP",
                                        "CLP", "PEN", "ECS", "ARS",
                                    };
vector<string> channelSubscriptions;
vector<string> api::getActiveSubscription(){
    return channelSubscriptions;
}
void api::registerSubscription(const string &index_name) {
    string subscription = "deribit_price_index." + index_name;
    if (find(channelSubscriptions.begin(), channelSubscriptions.end(), subscription) == channelSubscriptions.end()) {
        channelSubscriptions.push_back(subscription);
    }
}
bool api::removeActiveSubscription(const string &index_name) {
    string subscription_to_remove = "deribit_price_index." + index_name;
    auto it = find(channelSubscriptions.begin(), channelSubscriptions.end(), subscription_to_remove);
    if (it != channelSubscriptions.end()) {
        channelSubscriptions.erase(it);
        return 1;
    }
    return 0;
}
bool api::is_valid_instrument_name(const string& instrument) {
    regex instrument_pattern(R"(^[A-Z]{3,4}(-)(PERPETUAL|[0-9]{2}[A-Z]{3}[0-9]{2})$)");
    return regex_match(instrument, instrument_pattern);
}
string api::processRequest(const string &input) {
    map<string, function<string(string)>> action_map =
    {
        {"authorize", api::authenticateUser},
        {"sell", api::createSellOrder},
        {"buy", api::createBuyOrder},
        {"get_open_orders", api::fetchOpenOrders},
        {"modify", api::modifyOrder},
        {"cancel", api::cancelOrder},
        {"cancel_all", api::cancelAllOrders},
        {"positions", api::fetchPositions},
        {"orderbook", api::fetchOrderbook},
        {"subscribe", api::subscribeChannel},
        {"unsubscribe", api::unsubscribeChannel},
        {"unsubscribe_all", api::unsubscribeAllChannels}
    };
    istringstream s(input.substr(8));
    int id;
    string cmd;
    s >> id >> cmd;
    auto find = action_map.find(cmd);
    if (find == action_map.end()) {
        utils::printerr("ERROR: Unrecognized command. Please enter 'help' to see available commands.\n");
        return "";
    }
    return find->second(input.substr(8));
}
string api::authenticateUser(const string &input) {
    istringstream s(input);
    string auth;
    string id;
    string flag{""};
    string client_id;
    string secret;
    long long tm = utils::getCurrentTimestamp();
    s >> id >> auth >> client_id >> secret >> flag;
    if (client_id.empty()) {
        utils::printcmd("\n🔑 Client Authentication 🔑\n");
        utils::printcmd("Enter your client ID: ");
        cin >> client_id;
    }
    if (secret.empty() || flag == "-s") {
        utils::printcmd("Enter your client secret (input will be hidden): ");
        secret = utils::securePasswordInput();
    }
    string nonce = utils::generateRandomString(10);
    jsonrpc_request j;
    j["method"] = "public/auth";
    j["params"] = {{"grant_type", "client_credentials"},
                   {"client_id", client_id},
                   {"client_secret", secret},
                   {"timestamp", tm},
                   {"nonce", nonce},
                   {"scope", "session:name"}
                   };
    if (j.dump() != "") {
        AUTHENTICATION_SENT = true;
        vector<pair<string, string>> content = {
            {"Status", "Request Sent"},
            {"Client ID", client_id},
            {"Timestamp", to_string(tm)},
            {"Nonce", nonce},
            {"", ""},
            {"Message", "Authentication request sent to server"}
        };
        utils::displayBox("AUTHORIZATION REQUEST SENT", content,
                         fmt::rgb(0, 205, 102), "🔐");
        Credentials::password().setAccessToken("temp_" + client_id);
    } else {
        vector<pair<string, string>> errorContent = {
            {"Status", "Failed"},
            {"Error", "Could not create authorization request"},
            {"", ""},
            {"Message", "Please check your credentials and try again"}
        };
        utils::displayBox("AUTHORIZATION FAILED", errorContent,
                         fmt::rgb(255, 69, 0), "❌");
    }
    return j.dump();
}
string api::createSellOrder(const string &input) {
    string sell;
    string id;
    string instrument;
    string cmd;
    string access_key;
    string order_type;
    string label;
    string frc;
    int contracts{0};
    double amount{0.0};
    istringstream s(input);
    s >> id >> sell >> instrument >> label;
    if (Credentials::password().getAccessToken() == "") {
        utils::printcmd("Enter the access token: ");
        cin >> access_key;
    }
    else {
        access_key = Credentials::password().getAccessToken();
    }
    utils::printcmd("\n📊 Order Quantity Selection 📊");
    utils::printcmd("\nPlease choose how you want to specify the order quantity:");
    utils::printcmd("\n  [1] Number of contracts (e.g., 10 contracts)");
    utils::printcmd("\n  [2] Amount in currency (e.g., 0.1 BTC)");
    utils::printcmd("\nEnter your choice (1 or 2): ");
    int choice;
    cin >> choice;
    if (choice == 1) {
        utils::printcmd("\nEnter the number of contracts to sell: ");
        cin >> contracts;
    } else if (choice == 2) {
        utils::printcmd("\nEnter the amount of currency to sell: ");
        cin >> amount;
    } else {
        utils::printerr("\nInvalid choice. Please select either 1 (contracts) or 2 (amount).\n");
        return "";
    }
    vector<string> order_types = {
        "limit",
        "stop_limit",
        "take_limit",
        "market",
        "stop_market",
        "take_market",
        "market_limit",
        "trailing_stop"
    };
    map<string, vector<string>> order_type_tif = {
        {"limit", {"good_til_cancelled", "good_til_day", "fill_or_kill", "immediate_or_cancel"}},
        {"stop_limit", {"good_til_cancelled", "good_til_day", "fill_or_kill", "immediate_or_cancel"}},
        {"take_limit", {"good_til_cancelled", "good_til_day", "fill_or_kill", "immediate_or_cancel"}},
        {"market", {"good_til_cancelled", "good_til_day", "fill_or_kill", "immediate_or_cancel"}},
        {"stop_market", {"good_til_cancelled", "good_til_day", "fill_or_kill", "immediate_or_cancel"}},
        {"take_market", {"good_til_cancelled", "good_til_day", "fill_or_kill", "immediate_or_cancel"}},
        {"market_limit", {"good_til_cancelled", "good_til_day", "fill_or_kill", "immediate_or_cancel"}},
        {"trailing_stop", {"good_til_cancelled"}}
    };
    utils::printcmd("\n📝 Order Type Selection 📝");
    utils::printcmd("\nAvailable order types:");
    for (size_t i = 0; i < order_types.size(); ++i) {
        utils::printcmd("\n  [" + to_string(i + 1) + "] " + order_types[i]);
    }
    utils::printcmd("\n\nEnter the number corresponding to your desired order type (1-" +
                   to_string(order_types.size()) + "): ");
    int order_type_choice;
    cin >> order_type_choice;
    if (order_type_choice < 1 || order_type_choice > order_types.size()) {
        utils::printerr("\nInvalid selection. Please choose a number between 1 and " +
                       to_string(order_types.size()) + ".\n");
        return "";
    }
    order_type = order_types[order_type_choice - 1];
    const vector<string>& permitted_tif = order_type_tif[order_type];
    utils::printcmd("\n⏱️ Time-In-Force Selection ⏱️");
    utils::printcmd("\nAvailable time-in-force options for " + order_type + " order:");
    for (size_t i = 0; i < permitted_tif.size(); ++i) {
        utils::printcmd("\n  [" + to_string(i + 1) + "] " + permitted_tif[i]);
    }
    utils::printcmd("\n\nEnter the number corresponding to your preferred time-in-force option (1-" +
                   to_string(permitted_tif.size()) + "): ");
    int tif_choice;
    cin >> tif_choice;
    if (tif_choice < 1 || tif_choice > permitted_tif.size()) {
        utils::printerr("\nInvalid selection. Please choose a number between 1 and " +
                       to_string(permitted_tif.size()) + ".\n");
        return "";
    }
    frc = permitted_tif[tif_choice - 1];
    double price{0.0};
    if (order_type == "limit" || order_type == "stop_limit") {
        utils::printcmd("\n💰 Price Setting 💰");
        utils::printcmd("\nEnter the price at which you want to sell: ");
        cin >> price;
    }
    getPerformanceMonitor().start_measurement(PerformanceMonitor::ORDER_EXECUTION);
    jsonrpc_request j;
    j["method"] = "private/sell";
    j["params"] = {{"instrument_name", instrument},
                   {"access_token", access_key}};
    if (choice == 2 && amount > 0) {
        j["params"]["amount"] = amount;
    }
    else if (choice == 1 && contracts > 0) {
        j["params"]["contracts"] = contracts;
    }
    else {
        vector<pair<string, string>> errorContent = {
            {"Status", "Failed"},
            {"Error", "Invalid quantity specified"},
            {"", ""},
            {"Message", "Please specify a valid amount or number of contracts"}
        };
        utils::displayBox("ORDER CREATION FAILED", errorContent,
                         fmt::rgb(255, 69, 0), "❌");
        return "";
    }
    if (order_type == "limit" || order_type == "stop_limit" || order_type == "take_limit") {
        if (price <= 0) {
            vector<pair<string, string>> errorContent = {
                {"Status", "Failed"},
                {"Error", "Invalid price for " + order_type + " order"},
                {"", ""},
                {"Message", "Please specify a positive price for " + order_type + " orders"}
            };
            utils::displayBox("ORDER CREATION FAILED", errorContent,
                             fmt::rgb(255, 69, 0), "❌");
            return "";
        }
        j["params"]["price"] = price;
    } else if (price > 0) {
        j["params"]["price"] = price;
    }
    j["params"]["type"] = order_type;
    j["params"]["label"] = label;
    j["params"]["time_in_force"] = frc;
    getPerformanceMonitor().stop_measurement(PerformanceMonitor::ORDER_EXECUTION);
    string token = Credentials::password().getAccessToken();
    j["params"]["access_token"] = token;
    string json_request = j.dump();
    cout << "DEBUG: Sending sell order request: " << json_request << endl;
    if (token.empty() || token.substr(0, 5) == "temp_") {
        cout << "WARNING: No valid access token found. Authentication may be required." << endl;
        vector<pair<string, string>> errorContent = {
            {"Status", "Failed"},
            {"Error", "No valid access token"},
            {"", ""},
            {"Message", "Please authenticate first using 'deribit <id> authorize <client_id> <client_secret>'"}
        };
        utils::displayBox("ORDER CREATION FAILED", errorContent,
                         fmt::rgb(255, 69, 0), "❌");
        return "";
    }
    vector<pair<string, string>> content = {
        {"Instrument", instrument},
        {"Direction", "SELL"},
        {"Order Type", order_type},
        {"Time in Force", frc},
        {choice == 1 ? "Contracts" : "Amount", choice == 1 ? to_string(contracts) : to_string(amount)},
        {"Price", price > 0 ? to_string(price) : "Market Price"},
        {"Label", label.empty() ? "None" : label},
        {"", ""},
        {"Status", "Order created successfully - awaiting confirmation"}
    };
    utils::displayBox("SELL ORDER DETAILS", content,
                     fmt::rgb(255, 69, 0), "💸");
    return json_request;
}
string api::createBuyOrder(const string &input) {
    string buy;
    string id;
    string instrument;
    string cmd;
    string access_key;
    string order_type;
    string label;
    string frc;
    int contracts{0};
    double amount{0.0};
    istringstream s(input);
    s >> id >> buy >> instrument >> label;
    if (Credentials::password().getAccessToken() == "") {
        utils::printcmd("Enter the access token: ");
        cin >> access_key;
    }
    else {
        access_key = Credentials::password().getAccessToken();
    }
    utils::printcmd("\n📊 Order Quantity Selection 📊");
    utils::printcmd("\nPlease choose how you want to specify the order quantity:");
    utils::printcmd("\n  [1] Number of contracts (e.g., 10 contracts)");
    utils::printcmd("\n  [2] Amount in currency (e.g., 0.1 BTC)");
    utils::printcmd("\nEnter your choice (1 or 2): ");
    int choice;
    cin >> choice;
    if (choice == 1) {
        utils::printcmd("\nEnter the number of contracts to buy: ");
        cin >> contracts;
    } else if (choice == 2) {
        utils::printcmd("\nEnter the amount of currency to spend: ");
        cin >> amount;
    } else {
        utils::printerr("\nInvalid choice. Please select either 1 (contracts) or 2 (amount).\n");
        return "";
    }
    vector<string> order_types = {
        "limit",
        "stop_limit",
        "take_limit",
        "market",
        "stop_market",
        "take_market",
        "market_limit",
        "trailing_stop"
    };
    map<string, vector<string>> order_type_tif = {
        {"limit", {"good_til_cancelled", "good_til_day", "fill_or_kill", "immediate_or_cancel"}},
        {"stop_limit", {"good_til_cancelled", "good_til_day", "fill_or_kill", "immediate_or_cancel"}},
        {"take_limit", {"good_til_cancelled", "good_til_day", "fill_or_kill", "immediate_or_cancel"}},
        {"market", {"good_til_cancelled", "good_til_day", "fill_or_kill", "immediate_or_cancel"}},
        {"stop_market", {"good_til_cancelled", "good_til_day", "fill_or_kill", "immediate_or_cancel"}},
        {"take_market", {"good_til_cancelled", "good_til_day", "fill_or_kill", "immediate_or_cancel"}},
        {"market_limit", {"good_til_cancelled", "good_til_day", "fill_or_kill", "immediate_or_cancel"}},
        {"trailing_stop", {"good_til_cancelled"}}
    };
    utils::printcmd("\n📝 Order Type Selection 📝");
    utils::printcmd("\nAvailable order types:");
    for (size_t i = 0; i < order_types.size(); ++i) {
        utils::printcmd("\n  [" + to_string(i + 1) + "] " + order_types[i]);
    }
    utils::printcmd("\n\nEnter the number corresponding to your desired order type (1-" +
                   to_string(order_types.size()) + "): ");
    int order_type_choice;
    cin >> order_type_choice;
    if (order_type_choice < 1 || order_type_choice > order_types.size()) {
        utils::printerr("\nInvalid selection. Please choose a number between 1 and " +
                       to_string(order_types.size()) + ".\n");
        return "";
    }
    order_type = order_types[order_type_choice - 1];
    const vector<string>& permitted_tif = order_type_tif[order_type];
    utils::printcmd("\n⏱️ Time-In-Force Selection ⏱️");
    utils::printcmd("\nAvailable time-in-force options for " + order_type + " order:");
    for (size_t i = 0; i < permitted_tif.size(); ++i) {
        utils::printcmd("\n  [" + to_string(i + 1) + "] " + permitted_tif[i]);
    }
    utils::printcmd("\n\nEnter the number corresponding to your preferred time-in-force option (1-" +
                   to_string(permitted_tif.size()) + "): ");
    int tif_choice;
    cin >> tif_choice;
    if (tif_choice < 1 || tif_choice > permitted_tif.size()) {
        utils::printerr("\nInvalid selection. Please choose a number between 1 and " +
                       to_string(permitted_tif.size()) + ".\n");
        return "";
    }
    frc = permitted_tif[tif_choice - 1];
    double price{0.0};
    if (order_type == "limit" || order_type == "stop_limit") {
        utils::printcmd("\n💰 Price Setting 💰");
        utils::printcmd("\nEnter the price at which you want to buy: ");
        cin >> price;
    }
    getPerformanceMonitor().start_measurement(PerformanceMonitor::ORDER_EXECUTION);
    jsonrpc_request j;
    j["method"] = "private/buy";
    string token = Credentials::password().getAccessToken();
    j["params"] = {{"instrument_name", instrument},
                   {"access_token", token}};
    if (choice == 2 && amount > 0) {
        j["params"]["amount"] = amount;
    }
    else if (choice == 1 && contracts > 0) {
        j["params"]["contracts"] = contracts;
    }
    else {
        vector<pair<string, string>> errorContent = {
            {"Status", "Failed"},
            {"Error", "Invalid quantity specified"},
            {"", ""},
            {"Message", "Please specify a valid amount or number of contracts"}
        };
        utils::displayBox("ORDER CREATION FAILED", errorContent,
                         fmt::rgb(255, 69, 0), "❌");
        return "";
    }
    if (order_type == "limit" || order_type == "stop_limit" || order_type == "take_limit") {
        if (price <= 0) {
            vector<pair<string, string>> errorContent = {
                {"Status", "Failed"},
                {"Error", "Invalid price for " + order_type + " order"},
                {"", ""},
                {"Message", "Please specify a positive price for " + order_type + " orders"}
            };
            utils::displayBox("ORDER CREATION FAILED", errorContent,
                             fmt::rgb(255, 69, 0), "❌");
            return "";
        }
        j["params"]["price"] = price;
    } else if (price > 0) {
        j["params"]["price"] = price;
    }
    j["params"]["type"] = order_type;
    j["params"]["label"] = label;
    j["params"]["time_in_force"] = frc;
    getPerformanceMonitor().stop_measurement(PerformanceMonitor::ORDER_EXECUTION);
    string json_request = j.dump();
    cout << "DEBUG: Sending buy order request: " << json_request << endl;
    if (token.empty() || token.substr(0, 5) == "temp_") {
        cout << "WARNING: No valid access token found. Authentication may be required." << endl;
        vector<pair<string, string>> errorContent = {
            {"Status", "Failed"},
            {"Error", "No valid access token"},
            {"", ""},
            {"Message", "Please authenticate first using 'deribit <id> authorize <client_id> <client_secret>'"}
        };
        utils::displayBox("ORDER CREATION FAILED", errorContent,
                         fmt::rgb(255, 69, 0), "❌");
        return "";
    }
    vector<pair<string, string>> content = {
        {"Instrument", instrument},
        {"Direction", "BUY"},
        {"Order Type", order_type},
        {"Time in Force", frc},
        {choice == 1 ? "Contracts" : "Amount", choice == 1 ? to_string(contracts) : to_string(amount)},
        {"Price", price > 0 ? to_string(price) : "Market Price"},
        {"Label", label.empty() ? "None" : label},
        {"", ""},
        {"Status", "Order created successfully - awaiting confirmation"}
    };
    utils::displayBox("BUY ORDER DETAILS", content,
                     fmt::rgb(0, 255, 127), "💰");
    return json_request;
}
string api::modifyOrder(const string &input) {
    istringstream is(input);
    int id;
    string cmd;
    string ord_id;
    is >> id >> cmd >> ord_id;
    if (ord_id.empty()) {
        vector<pair<string, string>> errorContent = {
            {"Status", "Failed"},
            {"Error", "Order ID is required"},
            {"", ""},
            {"Message", "Please provide a valid order ID to modify"}
        };
        utils::displayBox("ORDER MODIFICATION FAILED", errorContent,
                         fmt::rgb(255, 69, 0), "❌");
        return "";
    }
    jsonrpc_request j;
    j["method"] = "private/edit";
    double amount = -1.0;
    double price = -1.0;
    utils::printcmd("\n💰 Order Price Modification 💰");
    utils::printcmd("\nEnter the new price (-1 to keep current price): ");
    cin >> price;
    utils::printcmd("\n📊 Order Amount Modification 📊");
    utils::printcmd("\nEnter the new amount (-1 to keep current amount): ");
    cin >> amount;
    getPerformanceMonitor().start_measurement(PerformanceMonitor::ORDER_EXECUTION);
    j["params"] = {{"order_id", ord_id}};
    if (amount > 0) j["params"]["amount"] = amount;
    if (price > 0) j["params"]["price"] = price;
    getPerformanceMonitor().stop_measurement(PerformanceMonitor::ORDER_EXECUTION);
    string json_request = j.dump();
    cout << "DEBUG: Sending modify order request: " << json_request << endl;
    string token = Credentials::password().getAccessToken();
    if (token.empty() || token.substr(0, 5) == "temp_") {
        cout << "WARNING: No valid access token found. Authentication may be required." << endl;
        vector<pair<string, string>> errorContent = {
            {"Status", "Failed"},
            {"Error", "No valid access token"},
            {"", ""},
            {"Message", "Please authenticate first using 'deribit <id> authorize <client_id> <client_secret>'"}
        };
        utils::displayBox("ORDER MODIFICATION FAILED", errorContent,
                         fmt::rgb(255, 69, 0), "❌");
        return "";
    }
    vector<pair<string, string>> content = {
        {"Order ID", ord_id},
        {"New Amount", amount > 0 ? to_string(amount) : "Unchanged"},
        {"New Price", price > 0 ? to_string(price) : "Unchanged"},
        {"", ""},
        {"Status", "Modification request sent - awaiting confirmation"}
    };
    utils::displayBox("ORDER MODIFICATION DETAILS", content,
                     fmt::rgb(255, 215, 0), "🔄");
    return json_request;
}
string api::cancelOrder(const string &input) {
    istringstream iss(input);
    int id;
    string cmd;
    string ord_id;
    iss >> id >> cmd >> ord_id;
    if (ord_id.empty()) {
        vector<pair<string, string>> errorContent = {
            {"Status", "Failed"},
            {"Error", "Order ID is required"},
            {"", ""},
            {"Message", "If you want to cancel all orders, use cancel_all instead"}
        };
        utils::displayBox("ORDER CANCELLATION FAILED", errorContent,
                         fmt::rgb(255, 69, 0), "❌");
        return "";
    }
    utils::printcmd("\n🚫 Order Cancellation Confirmation 🚫");
    utils::printcmd("\nYou are about to cancel order with ID: " + ord_id);
    utils::printcmd("\nEnter Y to proceed or any other key to abort: ");
    string confirmation;
    cin >> confirmation;
    if (confirmation != "Y" && confirmation != "y") {
        utils::printcmd("\nCancellation aborted by user.\n");
        return "";
    }
    getPerformanceMonitor().start_measurement(PerformanceMonitor::ORDER_EXECUTION);
    jsonrpc_request j;
    j["method"] = "private/cancel";
    j["params"]["order_id"] = ord_id;
    getPerformanceMonitor().stop_measurement(PerformanceMonitor::ORDER_EXECUTION);
    string json_request = j.dump();
    string token = Credentials::password().getAccessToken();
    if (token.empty() || token.substr(0, 5) == "temp_") {
        cout << "WARNING: No valid access token found. Authentication may be required." << endl;
        vector<pair<string, string>> errorContent = {
            {"Status", "Failed"},
            {"Error", "No valid access token"},
            {"", ""},
            {"Message", "Please authenticate first using 'deribit <id> authorize <client_id> <client_secret>'"}
        };
        utils::displayBox("ORDER CANCELLATION FAILED", errorContent,
                         fmt::rgb(255, 69, 0), "❌");
        return "";
    }
    vector<pair<string, string>> content = {
        {"Order ID", ord_id},
        {"", ""},
        {"Status", "Cancellation request sent"}
    };
    utils::displayBox("ORDER CANCELLATION DETAILS", content,
                     fmt::rgb(255, 99, 71), "🚫");
    return json_request;
}
string api::cancelAllOrders(const string &input) {
    istringstream iss(input);
    int id;
    string cmd;
    string option;
    string label;
    iss >> id >> cmd >> option >> label;
    if (option.empty()) {
        utils::printcmd("\n⚠️ Cancel All Orders Confirmation ⚠️");
        utils::printcmd("\nYou are about to cancel ALL orders on your account.");
        utils::printcmd("\nThis action cannot be undone.");
        utils::printcmd("\nEnter Y to proceed or any other key to abort: ");
        string confirmation;
        cin >> confirmation;
        if (confirmation != "Y" && confirmation != "y") {
            utils::printcmd("\nCancellation aborted by user.\n");
            return "";
        }
    }
    getPerformanceMonitor().start_measurement(PerformanceMonitor::ORDER_EXECUTION);
    jsonrpc_request j;
    j["params"] = {};
    vector<pair<string, string>> content;
    content.push_back({"", ""});
    string title = "CANCEL ALL ORDERS";
    string icon = "🧹";
    if (option.empty()) {
        j["method"] = "private/cancel_all";
        content.push_back({"Type", "All orders on account"});
    }
    else if (find(AVAILABLE_CURRENCIES.begin(), AVAILABLE_CURRENCIES.end(), option) == AVAILABLE_CURRENCIES.end()) {
        j["method"] = "private/cancel_all_by_instrument";
        j["params"]["instrument"] = option;
        title = "CANCEL ORDERS BY INSTRUMENT";
        content.push_back({"Instrument", option});
    }
    else if (option == "-s") {
        j["method"] = "private/cancel_by_label";
        j["params"]["label"] = label;
        title = "CANCEL ORDERS BY LABEL";
        content.push_back({"Label", label});
    }
    else {
        j["method"] = "private/cancel_all_by_currency";
        j["params"]["currency"] = option;
        title = "CANCEL ORDERS BY CURRENCY";
        content.push_back({"Currency", option});
    }
    content.push_back({"", ""});
    content.push_back({"Status", "Cancellation request sent - awaiting confirmation"});
    utils::displayBox(title, content, fmt::rgb(255, 99, 71), icon);
    getPerformanceMonitor().stop_measurement(PerformanceMonitor::ORDER_EXECUTION);
    string json_request = j.dump();
    cout << "DEBUG: Sending cancel all orders request: " << json_request << endl;
    string token = Credentials::password().getAccessToken();
    if (token.empty() || token.substr(0, 5) == "temp_") {
        cout << "WARNING: No valid access token found. Authentication may be required." << endl;
        vector<pair<string, string>> errorContent = {
            {"Status", "Failed"},
            {"Error", "No valid access token"},
            {"", ""},
            {"Message", "Please authenticate first using 'deribit <id> authorize <client_id> <client_secret>'"}
        };
        utils::displayBox("ORDER CANCELLATION FAILED", errorContent,
                         fmt::rgb(255, 69, 0), "❌");
        return "";
    }
    return json_request;
}
string api::fetchOpenOrders(const string &input) {
    getPerformanceMonitor().start_measurement(PerformanceMonitor::MARKET_DATA_HANDLING);
    istringstream is(input);
    int id;
    string cmd;
    string opt1;
    string opt2;
    is >> id >> cmd >> opt1 >> opt2;
    jsonrpc_request j;
    string access_token = Credentials::password().getAccessToken();
    if (access_token.empty() || access_token.substr(0, 5) == "temp_") {
        cout << "WARNING: No valid access token found. Authentication may be required." << endl;
        vector<pair<string, string>> errorContent = {
            {"Status", "Failed"},
            {"Error", "No valid access token"},
            {"", ""},
            {"Message", "Please authenticate first using 'deribit <id> authorize <client_id> <client_secret>'"}
        };
        utils::displayBox("FETCH ORDERS FAILED", errorContent,
                         fmt::rgb(255, 69, 0), "❌");
        return "";
    }
    vector<pair<string, string>> content;
    string title = "RETRIEVING OPEN ORDERS";
    string icon = "📋";
    fmt::rgb boxColor = fmt::rgb(0, 191, 255);
    j["params"] = {{"access_token", access_token}};
    if (opt1 == "") {
        j["method"] = "private/get_open_orders";
        content.push_back({"Query Type", "All open orders"});
    }
    else if (find(AVAILABLE_CURRENCIES.begin(), AVAILABLE_CURRENCIES.end(), opt1) == AVAILABLE_CURRENCIES.end()) {
        j["method"] = "private/get_open_orders_by_instrument";
        j["params"] = {{"instrument_name", opt1}, {"access_token", access_token}};
        content.push_back({"Query Type", "By instrument"});
        content.push_back({"Instrument", opt1});
    }
    else if (opt2 == "") {
        j["method"] = "private/get_open_orders_by_currency";
        j["params"] = {{"currency", opt1}, {"access_token", access_token}};
        content.push_back({"Query Type", "By currency"});
        content.push_back({"Currency", opt1});
    }
    else {
        j["method"] = "private/get_open_orders_by_label";
        j["params"] = {{"currency", opt1}, {"label", opt2}, {"access_token", access_token}};
        content.push_back({"Query Type", "By label"});
        content.push_back({"Currency", opt1});
        content.push_back({"Label", opt2});
    }
    content.push_back({"", ""});
    content.push_back({"Status", "Processing request..."});
    utils::displayBox(title, content, boxColor, icon);
    string json_request = j.dump();
    getPerformanceMonitor().stop_measurement(PerformanceMonitor::MARKET_DATA_HANDLING);
    fmt::print(fg(fmt::rgb(255, 215, 0)) | fmt::emphasis::bold, "\n🔍 Querying orders information... Results will appear below when received.\n\n");
    return json_request;
}
string api::fetchPositions(const string &input) {
    getPerformanceMonitor().start_measurement(PerformanceMonitor::MARKET_DATA_HANDLING);
    istringstream is(input);
    int id;
    string cmd;
    string currency;
    string kind;
    is >> id >> cmd >> currency >> kind;
    string access_token = Credentials::password().getAccessToken();
    if (access_token.empty() || access_token.substr(0, 5) == "temp_") {
        cout << "WARNING: No valid access token found. Authentication may be required." << endl;
        vector<pair<string, string>> errorContent = {
            {"Status", "Failed"},
            {"Error", "No valid access token"},
            {"", ""},
            {"Message", "Please authenticate first using 'deribit <id> authorize <client_id> <client_secret>'"}
        };
        utils::displayBox("FETCH POSITIONS FAILED", errorContent,
                         fmt::rgb(255, 69, 0), "❌");
        return "";
    }
    vector<pair<string, string>> content;
    string title = "RETRIEVING POSITIONS";
    string icon = "📊";
    fmt::rgb boxColor = fmt::rgb(102, 204, 255);
    if (!currency.empty()) {
        static const set<string> valid_currencies = {
            "BTC", "ETH", "USDC", "USDT", "EURR"
        };
        if (valid_currencies.find(currency) == valid_currencies.end()) {
            vector<pair<string, string>> errorContent = {
                {"Status", "Failed"},
                {"Error", "Invalid currency format"},
                {"", ""},
                {"Message", "Please use one of the supported currencies: BTC, ETH, USDC, USDT, EURR"}
            };
            utils::displayBox("INVALID CURRENCY", errorContent,
                           fmt::rgb(255, 69, 0), "❌");
            return "";
        }
        content.push_back({"Currency", currency});
    } else {
        content.push_back({"Currency", "All currencies"});
    }
    if (!kind.empty()) {
        static const set<string> valid_kinds = {
            "future", "option", "spot",
            "future_combo", "option_combo"
        };
        if (valid_kinds.find(kind) == valid_kinds.end()) {
            vector<pair<string, string>> errorContent = {
                {"Status", "Failed"},
                {"Error", "Invalid instrument kind"},
                {"", ""},
                {"Message", "Please use one of the supported kinds: future, option, spot, future_combo, option_combo"}
            };
            utils::displayBox("INVALID INSTRUMENT KIND", errorContent,
                           fmt::rgb(255, 69, 0), "❌");
            return "";
        }
        content.push_back({"Instrument Type", kind});
    } else {
        content.push_back({"Instrument Type", "All types"});
    }
    content.push_back({"", ""});
    content.push_back({"Status", "Processing request..."});
    utils::displayBox(title, content, boxColor, icon);
    jsonrpc_request j;
    j["method"] = "private/get_positions";
    j["params"]["access_token"] = access_token;
    if (!currency.empty()) {
        j["params"]["currency"] = currency;
    }
    if (!kind.empty()) {
        j["params"]["kind"] = kind;
    }
    string json_request = j.dump();
    cout << "DEBUG: Sending get positions request: " << json_request << endl;
    getPerformanceMonitor().stop_measurement(PerformanceMonitor::MARKET_DATA_HANDLING);
    fmt::print(fg(fmt::rgb(255, 215, 0)) | fmt::emphasis::bold, "\n🔍 Querying positions information... Results will appear below when received.\n\n");
    return json_request;
}
string api::fetchOrderbook(const string &input) {
    getPerformanceMonitor().start_measurement(PerformanceMonitor::MARKET_DATA_HANDLING);
    istringstream is(input);
    int id;
    string cmd;
    string instrument;
    string depth_str;
    int depth = 10;
    is >> id >> cmd >> instrument >> depth_str;
    if (instrument.empty()) {
        vector<pair<string, string>> errorContent = {
            {"Status", "Failed"},
            {"Error", "Missing instrument name"},
            {"", ""},
            {"Message", "Please specify an instrument name (e.g., BTC-PERPETUAL)"}
        };
        utils::displayBox("ORDERBOOK REQUEST FAILED", errorContent,
                         fmt::rgb(255, 69, 0), "❌");
        return "";
    }
    if (!depth_str.empty()) {
        try {
            depth = stoi(depth_str);
            if (depth < 1 || depth > 100) {
                depth = 10;
            }
        } catch (...) {
        }
    }
    vector<pair<string, string>> content = {
        {"Instrument", instrument},
        {"Depth", to_string(depth)},
        {"", ""},
        {"Status", "Processing request..."}
    };
    utils::displayBox("RETRIEVING ORDERBOOK", content,
                     fmt::rgb(64, 224, 208), "📚");
    jsonrpc_request j;
    j["method"] = "public/get_order_book";
    j["params"] = {
        {"instrument_name", instrument},
        {"depth", depth}
    };
    string json_request = j.dump();
    cout << "DEBUG: Sending get orderbook request: " << json_request << endl;
    getPerformanceMonitor().stop_measurement(PerformanceMonitor::MARKET_DATA_HANDLING);
    fmt::print(fg(fmt::rgb(255, 215, 0)) | fmt::emphasis::bold, "\n🔍 Querying orderbook information for {}... Results will appear below when received.\n\n", instrument);
    return json_request;
}
string api::subscribeChannel(const string &input) {
    istringstream is(input);
    int id;
    string cmd;
    string index_name;
    is >> id >> cmd >> index_name;
    if (index_name.empty()) {
        vector<pair<string, string>> errorContent = {
            {"Status", "Failed"},
            {"Error", "Missing index name"},
            {"", ""},
            {"Message", "Please specify an index name to subscribe to (e.g., BTC, ETH)"}
        };
        utils::displayBox("SUBSCRIPTION FAILED", errorContent,
                         fmt::rgb(255, 69, 0), "❌");
        return "";
    }
    registerSubscription(index_name);
    vector<pair<string, string>> content = {
        {"Index Name", index_name},
        {"Channel", "deribit_price_index." + index_name},
        {"", ""},
        {"Status", "Successfully subscribed to price updates"}
    };
    utils::displayBox("SUBSCRIPTION SUCCESSFUL", content,
                     fmt::rgb(0, 255, 127), "📊");
    return "";
}
string api::unsubscribeChannel(const string &input) {
    istringstream is(input);
    int id;
    string cmd;
    string index_name;
    is >> id >> cmd >> index_name;
    if (index_name.empty()) {
        vector<pair<string, string>> errorContent = {
            {"Status", "Failed"},
            {"Error", "Missing index name"},
            {"", ""},
            {"Message", "Please specify an index name to unsubscribe from"}
        };
        utils::displayBox("UNSUBSCRIPTION FAILED", errorContent,
                         fmt::rgb(255, 69, 0), "❌");
        return "";
    }
    if(!removeActiveSubscription(index_name)) {
        vector<pair<string, string>> errorContent = {
            {"Status", "Failed"},
            {"Error", "Not subscribed to this index"},
            {"Index Name", index_name},
            {"", ""},
            {"Message", "You can only unsubscribe from indexes you are subscribed to"}
        };
        utils::displayBox("UNSUBSCRIPTION FAILED", errorContent,
                         fmt::rgb(255, 69, 0), "❌");
    } else {
        vector<pair<string, string>> content = {
            {"Index Name", index_name},
            {"Channel", "deribit_price_index." + index_name},
            {"", ""},
            {"Status", "Successfully unsubscribed from price updates"},
            {"Remaining Subscriptions", to_string(channelSubscriptions.size())}
        };
        utils::displayBox("UNSUBSCRIPTION SUCCESSFUL", content,
                         fmt::rgb(255, 215, 0), "🔕");
    }
    return "";
}
string api::unsubscribeAllChannels(const string &input) {
    istringstream is(input);
    int id;
    string cmd;
    int previous_count = channelSubscriptions.size();
    if (previous_count == 0) {
        vector<pair<string, string>> infoContent = {
            {"Status", "No action taken"},
            {"", ""},
            {"Message", "No active subscriptions to remove"}
        };
        utils::displayBox("NO ACTIVE SUBSCRIPTIONS", infoContent,
                         fmt::rgb(64, 224, 208), "ℹ️");
        return "";
    }
    channelSubscriptions = {};
    vector<pair<string, string>> content = {
        {"Previous Subscriptions", to_string(previous_count)},
        {"Current Subscriptions", "0"},
        {"", ""},
        {"Status", "Successfully unsubscribed from all price updates"}
    };
    utils::displayBox("ALL SUBSCRIPTIONS REMOVED", content,
                     fmt::rgb(255, 165, 0), "🧹");
    return "";
}