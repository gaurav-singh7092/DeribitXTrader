#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H
#include <map>
#include <string>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <thread>
#include <memory>
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXNetSystem.h>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
using namespace std;
extern bool AUTHENTICATION_SENT;
extern bool isDataStreaming;
class SocketEndpoint;
class ConnectionDetails {
private:
    int m_connection_id;
    string m_connection_status;
    string m_endpoint_uri;
    string m_server_info;
    string m_error_message;
    vector<string> m_transaction_logs;
    std::unique_ptr<ix::WebSocket> m_webSocketClient;
    SocketEndpoint* m_endpoint_controller;
public:
    typedef shared_ptr<ConnectionDetails> ptr;
    mutex connection_mutex;
    condition_variable connection_cv;
    vector<string> m_received_data;
    bool DATA_PROCESSED;
    ConnectionDetails(int id, string uri, SocketEndpoint* endpoint = nullptr);
    ~ConnectionDetails();
    int get_id();
    string get_status();
    string get_uri() const { return m_endpoint_uri; }
    string get_server() const { return m_server_info; }
    string get_error_reason() const { return m_error_message; }
    void record_sent_message(string const &message);
    void record_summary(string const &message, string const &sent);
    void setup_websocket();
    void close(uint16_t code = 1000, const string& reason = "");
    bool send(const string& message);
    ix::WebSocket* get_websocket();
    friend ostream &operator<< (ostream &out, ConnectionDetails const &data);
};
class SocketEndpoint {
private:
    typedef map<int, ConnectionDetails::ptr> connection_list;
    connection_list m_active_connections;
    int m_next_id;
public:
    SocketEndpoint();
    ~SocketEndpoint();
    int connect(string const &uri);
    ConnectionDetails::ptr get_metadata(int id) const;
    void close(int id, uint16_t code = 1000, string reason = "");
    int send(int id, string message);
    int streamSubscriptions(const vector<string>& connections);
};
#endif 