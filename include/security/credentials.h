#pragma once
#include <string>
using namespace std;
class Credentials {
    private:
        static int instance_count;
        string auth_token;
        Credentials() : auth_token("") {}
    public:
        static Credentials &password();
        Credentials(const Credentials&) = delete;
        void operator=(const Credentials&) = delete;
        void setAccessToken(const string& token);
        void setAccessToken(int& token);
        string getAccessToken() const;
};
