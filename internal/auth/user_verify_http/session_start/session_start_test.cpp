#include <gtest/gtest.h>
#include <pqxx/pqxx>
#include <sw/redis++/redis.h>
#include <nlohmann/json.hpp>
#include "../../user_verify/verification/user_verify.h"
#include "session_start.h"

const std::string PG_CONN_STR = 
    "host=localhost "
    "port=5432 "
    "user=admin "
    "password=secret "
    "dbname=timmipay "
    "sslmode=disable";

const std::string REDIS_CONN_STR = "tcp://localhost:6379";

class ProdSessionTest : public ::testing::Test {
protected:
    pqxx::connection pg_conn{PG_CONN_STR};
    sw::redis::Redis redis_conn{REDIS_CONN_STR};
    UserVerifier verifier{pg_conn, redis_conn};
    
    std::string test_email = "test_user@example.com";
    std::string test_hash = "5f4dcc3b5aa765d61d8327deb882cf99";
    std::string test_username = "aboba";

    void SetUp() override {
        pqxx::work txn(pg_conn);
        txn.exec_params(
            "INSERT INTO users (email, password_hash, username) VALUES ($1, $2, $3) "
            "ON CONFLICT (email) DO NOTHING",
            test_email, test_hash, test_username);
        txn.commit();
    }

    void TearDown() override {
        pqxx::work txn(pg_conn);
        txn.exec_params("DELETE FROM users WHERE email = $1", test_email);
        txn.commit();
    }
};

TEST_F(ProdSessionTest, RealDbIntegration) {
    SessionStart session_handler(verifier);
    
    nlohmann::json request = {
        {"email", test_email},
        {"password_hash", test_hash}
    };
    
    nlohmann::json response = session_handler.HandleRequest(request);
    
    EXPECT_FALSE(response["token"].empty());
    EXPECT_FALSE(response.contains("error"));
}
