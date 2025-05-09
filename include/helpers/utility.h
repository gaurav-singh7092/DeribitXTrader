#pragma once
#include <iostream>
#include <fmt/color.h>
#include <fmt/core.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <map>
#include <vector>
using namespace std;
namespace utils {
    long long getCurrentTimestamp();
    string generateRandomString(const int len);
    string createSignature(long long timestamp, string nonce, string data, string clientsecret);
    string convertToHexString(const unsigned char* data, unsigned int length);
    string generateHmacSha256(const string& key, const string& data);
    string formatJson(string j);
    string mapToString(map<string, string> mpp);
    string securePasswordInput();
    void printcmd(string const &str);
    void printcmd(string const &str, int r, int g, int b);
    void printerr(string const &str);
    void printsuccess(string const &str);
    void printinfo(string const &str);
    void printwarning(string const &str);
    void printOrderbook(const string &instrument, const string &data, int depth = 10);
    void printPositions(const string &data);
    void printOpenOrders(const string &data);
    void printTradeConfirmation(const string &data);
    void printSubscriptionStatus(const vector<string> &subscriptions);
    void printLatencyReport(const map<string, double> &latencyData);
    void displayBox(const string &title, const vector<pair<string, string>> &content, 
                   fmt::rgb boxColor = fmt::rgb(0, 255, 127), 
                   const string &icon = "✅");
    int getTerminalWidth();
    void printHeader();
    void printHelp();
    void printStatusBar(const string &status, bool isConnected = false);
    void drawProgressBar(int percent);
    void clear_console();
    bool is_key_pressed(char key);
    bool check_key_pressed(char key);
}
