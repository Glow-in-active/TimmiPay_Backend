#include <gtest/gtest.h>
#include <pqxx/pqxx>
#include <sw/redis++/redis.h>
#include <nlohmann/json.hpp>

#include "balance_viewer.h"
#include "../../storage/users_balance/users_balance.h"
#include "../../storage/user_verify/redis_get/redis_get_id_by_token.h"
#include "../../uuid_generator/uuid_generator.h"

#include "../../storage/config/config.h"
#include "../../storage/redis_config/config_redis.h"
#include "../../storage/postgres_connect/connect.h"
#include "../../storage/redis_connect/connect_redis.h"

class BalanceViewerTest : public ::testing::Test {
protected:
    pqxx::connection pg_conn;
    sw::redis::Redis redis_conn;
    BalanceStorage balance_storage;
    BalanceViewer balance_viewer;
    UUIDGenerator uuid_gen;

    std::string test_user_id;
    std::string test_token = "valid_token_123";
    std::string invalid_token = "invalid_token_456";
    
    const std::string currency_usd_id = "00000000-0000-0000-0000-000000000001";
    const std::string currency_eur_id = "00000000-0000-0000-0000-000000000002";

    const std::string test_email = "test_user@example.com";
    const std::string test_password_hash = "5f4dcc3b5aa765d61d8327deb882cf99";
    const std::string test_username = "test_user";

    BalanceViewerTest()
        : pg_conn(connect_to_database(load_config("database_config/test_postgres_config.json"))),
          redis_conn(connect_to_redis(load_redis_config("database_config/test_redis_config.json"))),
          balance_storage(pg_conn),
          balance_viewer(redis_conn, balance_storage) 
    {
        test_user_id = uuid_gen.generateUUID();
    }

    void SetUp() override {
        pqxx::work txn(pg_conn);
        
        txn.exec_params("DELETE FROM accounts WHERE user_id = $1", test_user_id);
        txn.exec_params("DELETE FROM users WHERE id = $1", test_user_id);
        txn.exec_params("DELETE FROM currencies WHERE id IN ($1, $2)", currency_usd_id, currency_eur_id);
        
        txn.exec_params(
            "INSERT INTO users (id, email, password_hash, username) VALUES "
            "($1, $2, $3, $4)",
            test_user_id, test_email, test_password_hash, test_username);
        
        txn.exec_params(
            "INSERT INTO currencies (id, code, name) VALUES "
            "($1, 'TUS', 'Test USD'), ($2, 'TEU', 'Test EUR') "
            "ON CONFLICT (id) DO UPDATE SET code = EXCLUDED.code",
            currency_usd_id, currency_eur_id);
            
        txn.exec_params(
            "INSERT INTO accounts (user_id, currency_id, balance) VALUES "
            "($1, $2, 100.5), ($1, $3, 50.0)",
            test_user_id, currency_usd_id, currency_eur_id);
            
        txn.commit();

        redis_conn.hset(test_token, "id", test_user_id);
    }

    void TearDown() override {
        redis_conn.del(test_token);
        redis_conn.del(invalid_token);

        pqxx::work txn(pg_conn);
        txn.exec_params("DELETE FROM accounts WHERE user_id = $1", test_user_id);
        txn.exec_params("DELETE FROM users WHERE id = $1", test_user_id);
        txn.exec_params("DELETE FROM currencies WHERE id IN ($1, $2)", currency_usd_id, currency_eur_id);
        txn.commit();
    }
};

TEST_F(BalanceViewerTest, ValidTokenReturnsBalances) {
    nlohmann::json request = {{"token", test_token}};
    auto response = balance_viewer.HandleRequest(request);
    
    EXPECT_TRUE(response.contains("balances"));
    EXPECT_DOUBLE_EQ(response["balances"]["TUS"].get<double>(), 100.5);
    EXPECT_DOUBLE_EQ(response["balances"]["TEU"].get<double>(), 50.0);
    EXPECT_FALSE(response.contains("error"));
}

TEST_F(BalanceViewerTest, InvalidTokenReturnsError) {
    nlohmann::json request = {{"token", invalid_token}};
    auto response = balance_viewer.HandleRequest(request);
    
    EXPECT_EQ(
        response["error"].get<std::string>(), 
        "System error: Token not found or id field missing"
    );
}

TEST_F(BalanceViewerTest, MissingTokenReturnsError) {
    nlohmann::json request = {{"wrong_key", "value"}};
    auto response = balance_viewer.HandleRequest(request);
    
    EXPECT_TRUE(response.contains("error"));
    EXPECT_EQ(response["error"].get<std::string>(), "Invalid JSON format");
    EXPECT_TRUE(response.contains("details"));
}

TEST_F(BalanceViewerTest, EmptyBalancesReturnsEmptyObject) {
    pqxx::work txn(pg_conn);
    txn.exec_params("DELETE FROM accounts WHERE user_id = $1", test_user_id);
    txn.commit();

    nlohmann::json request = {{"token", test_token}};
    auto response = balance_viewer.HandleRequest(request);
    
    EXPECT_TRUE(response["balances"].empty());
}