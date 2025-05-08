#include <gtest/gtest.h>
#include "balance_viewer_endpoint.h"
#include "../logic/balance_viewer.h"
#include "../../storage/users_balance/users_balance.h"
#include <sw/redis++/redis++.h>
#include <pqxx/pqxx>
#include <nlohmann/json.hpp>
#include <crow.h>
#include <sstream>
#include <random>
#include <chrono>

// Генератор валидных UUID
std::string generate_fake_uuid() {
    std::stringstream ss;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::uniform_int_distribution<> dis2(8, 11);

    const char* hex_chars = "0123456789abcdef";
    
    // Генерация по стандарту RFC 4122
    for (int i = 0; i < 8; i++) ss << hex_chars[dis(gen)];
    ss << '-';
    for (int i = 0; i < 4; i++) ss << hex_chars[dis(gen)];
    ss << "-4"; // Версия 4
    for (int i = 0; i < 3; i++) ss << hex_chars[dis(gen)];
    ss << '-';
    ss << hex_chars[dis2(gen)]; // Вариант 8-Б
    for (int i = 0; i < 3; i++) ss << hex_chars[dis(gen)];
    ss << '-';
    for (int i = 0; i < 12; i++) ss << hex_chars[dis(gen)];
    
    return ss.str();
}

class BalanceViewerIntegrationTest : public ::testing::Test {
protected:
    std::unique_ptr<sw::redis::Redis> redis;
    std::unique_ptr<BalanceStorage> balance_storage;
    std::unique_ptr<BalanceViewer> balance_viewer;
    decltype(create_balance_viewer_handler(*balance_viewer)) handler;

    std::string test_token = "test-token";
    std::string test_user_id;
    pqxx::connection* pg_conn;

    void SetUp() override {
        test_user_id = generate_fake_uuid();

        sw::redis::ConnectionOptions redis_opts;
        redis_opts.host = "localhost";
        redis_opts.port = 6380;
        redis = std::make_unique<sw::redis::Redis>(redis_opts);

        // Сохраняем UUID как строку
        redis->set("auth:" + test_token, test_user_id);

        pg_conn = new pqxx::connection{
            "host=localhost port=5433 dbname=timmipay_test user=admin password=secret sslmode=disable"
        };

        balance_storage = std::make_unique<BalanceStorage>(*pg_conn);
        balance_viewer = std::make_unique<BalanceViewer>(*redis, *balance_storage);
        handler = create_balance_viewer_handler(*balance_viewer);

        pqxx::work txn(*pg_conn);

        // Явное создание типа UUID если не существует
        txn.exec0("CREATE EXTENSION IF NOT EXISTS \"uuid-ossp\"");
        txn.exec0("CREATE TABLE IF NOT EXISTS accounts ("
                  "id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),"
                  "user_id UUID NOT NULL,"
                  "currency_id INTEGER REFERENCES currencies(id),"
                  "balance DOUBLE PRECISION NOT NULL)");

        // Вставка с явным приведением типа
        txn.exec_params(
            "INSERT INTO accounts (user_id, currency_id, balance) "
            "VALUES ($1::uuid, (SELECT id FROM currencies WHERE code = 'USD'), 500.0)",
            test_user_id
        );
        
        txn.exec_params(
            "INSERT INTO accounts (user_id, currency_id, balance) "
            "VALUES ($1::uuid, (SELECT id FROM currencies WHERE code = 'BTC'), 0.01)",
            test_user_id
        );
        txn.commit();
    }

    void TearDown() override {
        pqxx::work txn(*pg_conn);
        txn.exec_params("DELETE FROM accounts WHERE user_id = $1::uuid", test_user_id);
        txn.commit();

        redis->del("auth:" + test_token);
        delete pg_conn;
    }
};

// Тесты остаются без изменений

TEST_F(BalanceViewerIntegrationTest, ReturnsInsertedBalancesCorrectly) {
    crow::request req;
    req.body = R"({"token": "test-token"})";

    auto res = handler(req);
    EXPECT_EQ(res.code, 200);

    auto json = nlohmann::json::parse(res.body);
    ASSERT_TRUE(json.contains("balances"));
    EXPECT_DOUBLE_EQ(json["balances"]["USD"], 500.0);
    EXPECT_DOUBLE_EQ(json["balances"]["BTC"], 0.01);
}

TEST_F(BalanceViewerIntegrationTest, ReturnsEmptyBalancesIfNoneExist) {
    // Используем нового тестового пользователя без аккаунтов
    std::string empty_user_id = generate_fake_uuid();  // Новый UUID
    std::string empty_token = "empty-token";

    redis->set("auth:" + empty_token, empty_user_id);

    crow::request req;
    req.body = R"({"token": "empty-token"})";
    auto res = handler(req);
    redis->del("auth:" + empty_token);

    EXPECT_EQ(res.code, 200);
    auto json = nlohmann::json::parse(res.body);
    ASSERT_TRUE(json.contains("balances"));
    EXPECT_TRUE(json["balances"].empty());
}

TEST_F(BalanceViewerIntegrationTest, Returns401IfTokenNotFound) {
    crow::request req;
    req.body = R"({"token": "nonexistent-token"})";

    auto res = handler(req);
    EXPECT_EQ(res.code, 401);

    auto json = nlohmann::json::parse(res.body);
    ASSERT_EQ(json["error"], "Token not found");
}

TEST_F(BalanceViewerIntegrationTest, Returns400IfMalformedJson) {
    crow::request req;
    req.body = R"({invalid-json)";  // malformed JSON

    auto res = handler(req);
    EXPECT_EQ(res.code, 400);

    auto json = nlohmann::json::parse(res.body);
    ASSERT_EQ(json["error"], "Invalid JSON format");
    ASSERT_TRUE(json.contains("details"));
}

TEST_F(BalanceViewerIntegrationTest, Returns400IfTokenMissingInJson) {
    crow::request req;
    req.body = R"({"wrong_key": "value"})";

    auto res = handler(req);
    EXPECT_EQ(res.code, 400);

    auto json = nlohmann::json::parse(res.body);
    ASSERT_EQ(json["error"], "Invalid JSON format");
    ASSERT_TRUE(json["details"].is_string());
}
