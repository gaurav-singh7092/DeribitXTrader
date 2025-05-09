#include <gtest/gtest.h>
#include "security/credentials.h"
#include <string>


class CredentialsTest : public ::testing::Test {
protected:
    void SetUp() override {
        
    }

    void TearDown() override {
        
    }
};


TEST_F(CredentialsTest, AccessTokenHandling) {
    
    std::string initialToken = Credentials::password().getAccessToken();

    
    if (initialToken.empty()) {
        const std::string testToken = "test_access_token_12345";
        Credentials::password().setAccessToken(testToken);

        
        EXPECT_EQ(Credentials::password().getAccessToken(), testToken);

        
        
    } else {
        
        EXPECT_FALSE(initialToken.empty());

        
        Credentials::password().setAccessToken("new_token");
        EXPECT_EQ(Credentials::password().getAccessToken(), initialToken);
    }
}


TEST_F(CredentialsTest, SingletonBehavior) {
    
    Credentials& cred1 = Credentials::password();
    Credentials& cred2 = Credentials::password();

    
    EXPECT_EQ(&cred1, &cred2);

    
    EXPECT_EQ(cred1.getAccessToken(), cred2.getAccessToken());

    
    
}
