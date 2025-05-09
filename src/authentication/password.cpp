#include <iostream>
#include <string>
#include "authentication/password.h"
using namespace std;
int Credentials::instance_count = 0;
Credentials &Credentials::password() {
    static Credentials cred;
    return cred;
}
void Credentials::setAccessToken(const string& token) {
    if (token.substr(0, 5) == "temp_") {
        auth_token = token;
        return;
    }
    if(instance_count > 1 && auth_token.substr(0, 5) != "temp_") {
        cout << "WARNING: Access token can only be set once per session" << endl;
        return;
    }
    auth_token = token;
    if (auth_token.substr(0, 5) != "temp_") {
        instance_count++;
    }
    cout << "DEBUG: Access token updated" << endl;
}
void Credentials::setAccessToken(int& token) {
    setAccessToken(to_string(token));
}
string Credentials::getAccessToken() const {
    return auth_token;
}
