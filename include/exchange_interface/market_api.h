#pragma once
#include "data_format/json_parser.hpp"
#include <string>
#include <vector>
using namespace std;
using json = nlohmann::json;
extern bool AUTHENTICATION_SENT;
extern vector<string> AVAILABLE_CURRENCIES;
extern vector<string> channelSubscriptions;
class jsonrpc_request : public json {
    public:
        jsonrpc_request(){
            (*this)["jsonrpc"] = "2.0";
            srand( time(NULL) );
            long requestId = rand();
            (*this)["id"] = requestId;
        }
        jsonrpc_request(const string& methodName){
            (*this)["jsonrpc"] = "2.0";
            (*this)["method"] = methodName;
            srand( time(NULL) );
            long requestId = rand();
            (*this)["id"] = requestId;
        }
};
namespace api {
    vector<string> getActiveSubscription();
    bool is_valid_instrument_name(const string& instrument);
    void registerSubscription(const string &index_name);
    bool removeActiveSubscription(const string &index_name);
    string processRequest(const string &input);
    string authenticateUser(const string &cmd);
    string createSellOrder(const string &input);
    string createBuyOrder(const string &input);
    string fetchOpenOrders(const string &input);
    string modifyOrder(const string &input);
    string cancelOrder(const string &input);
    string cancelAllOrders(const string &input);
    string fetchPositions(const string &input);
    string fetchOrderbook(const string &input);
    string subscribeChannel(const string &input);
    string unsubscribeChannel(const string &input);
    string unsubscribeAllChannels(const string &input);
}
