#include <gtest/gtest.h>
#include <pqxx/pqxx>
#include <sw/redis++/redis.h>
#include <nlohmann/json.hpp>

#include "../../user_verify/verification/user_verify.h"
#include "session_start.h"

#include "../../../../storage/config/config.h"
#include "../../../../storage/redis_config/config_redis.h"
#include "../../../../storage/postgres_connect/connect.h"
#include "../../../../storage/redis_connect/connect_redis.h"

class ProdSessionTest : public ::testing::Test {
protected:
    pqxx::connection pg_conn;
    sw::redis::Redis redis_conn;
    UserVerifier verifier;

    std::string test_email = "test_user@example.com";
    std::string test_hash = "5f4dcc3b5aa765d61d8327deb882cf99";
    std::string test_username = "aboba";

    ProdSessionTest()
        : pg_conn(connect_to_database(load_config("database_config/test_postgres_config.json"))),
          redis_conn(connect_to_redis(load_redis_config("database_config/test_redis_config.json"))),
          verifier(pg_conn, redis_conn) {}

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
