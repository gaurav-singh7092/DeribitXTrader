#include <iostream>
#include <string>
#include "security/credentials.h"
using namespace std;
int Credentials::instance_count = 0;  
Credentials &Credentials::password() {
    static Credentials cred;
    return cred;
}
void Credentials::setAccessToken(const string& token) {
    if(instance_count > 1) {
        cout << "WARNING: Access token can only be set once per session" << endl;
        return;
    }
    auth_token = token;
    instance_count++;
}
void Credentials::setAccessToken(int& token) {
    if(instance_count > 1) {
        cout << "WARNING: Access token can only be set once per session" << endl;
        return;
    }
    auth_token = to_string(token);
    instance_count++;
}
string Credentials::getAccessToken() const {
    return auth_token;
}
