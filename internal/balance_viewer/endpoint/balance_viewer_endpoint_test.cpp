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

#include "../../storage/config/config.h"
#include "../../storage/redis_config/config_redis.h"
#include "../../storage/postgres_connect/connect.h"
#include "../../storage/redis_connect/connect_redis.h"
#include "../../uuid_generator/uuid_generator.h"

class BalanceViewerIntegrationTest : public ::testing::Test {
protected:
    sw::redis::Redis redis_conn;
    std::unique_ptr<pqxx::connection> pg_conn;
    std::unique_ptr<BalanceStorage> balance_storage;
    std::unique_ptr<BalanceViewer> balance_viewer;
    decltype(create_balance_viewer_handler(*balance_viewer)) handler;

    std::string test_token = "test-token";
    std::string test_user_id; // Снова не константный
    UUIDGenerator uuid_gen;   // Возвращаем генератор

    std::string currency_usd_id; // Теперь не константный
    std::string currency_btc_id; // Теперь не константный

    std::string empty_user_id; // Добавляем сюда
    std::string empty_token;   // Добавляем сюда

    BalanceViewerIntegrationTest()
        : redis_conn(connect_to_redis(load_redis_config("database_config/test_redis_config.json"))) // pg_conn инициализируется в SetUp
    {}

    void SetUp() override {
        // Очищаем Redis перед каждым тестом
        redis_conn.flushdb();

        pg_conn = std::make_unique<pqxx::connection>(connect_to_database(load_config("database_config/test_postgres_config.json"))); // Инициализируем соединение здесь
        balance_storage = std::make_unique<BalanceStorage>(*pg_conn); // Передаем разыменованный указатель
        balance_viewer = std::make_unique<BalanceViewer>(redis_conn, *balance_storage); // Передаем разыменованный указатель

        test_user_id = uuid_gen.generateUUID(); // Генерируем UUID пользователя
        currency_usd_id = uuid_gen.generateUUID(); // Генерируем UUID для USD
        currency_btc_id = uuid_gen.generateUUID(); // Генерируем UUID для BTC

        empty_user_id = uuid_gen.generateUUID(); // Генерируем UUID для пустого пользователя
        empty_token = "empty-token-" + empty_user_id; // Уникальный токен

        // Настройка Redis
        redis_conn.hset(test_token, "id", test_user_id);
        redis_conn.hset(empty_token, "id", empty_user_id); // Настройка Redis для пустого пользователя

        // Инициализация зависимостей
        handler = create_balance_viewer_handler(*balance_viewer);

        pqxx::work txn(*pg_conn); // Начинаем основную транзакцию для настройки
        
        // Очищаем таблицу currencies в рамках этой же транзакции
        txn.exec0("DELETE FROM currencies;"); 

        // Создание тестового пользователя
        txn.exec_params(
            "INSERT INTO users (id, username, email, password_hash) "
            "VALUES ($1::uuid, 'test_user', 'test@example.com', 'test_password_hash') "
            "ON CONFLICT (id) DO NOTHING",
            test_user_id
        );

        // Добавление валют
        txn.exec_params(
            "INSERT INTO currencies (id, code, name) "
            "VALUES ($1::uuid, 'USD', 'US Dollar'), ($2::uuid, 'BTC', 'Bitcoin') "
            "ON CONFLICT (code) DO NOTHING",
            currency_usd_id, currency_btc_id
        );

        // Добавление аккаунтов
        txn.exec_params(
            "INSERT INTO accounts (user_id, currency_id, balance) "
            "VALUES ($1::uuid, $2::uuid, 500.0)",
            test_user_id, currency_usd_id
        );
        
        txn.exec_params(
            "INSERT INTO accounts (user_id, currency_id, balance) "
            "VALUES ($1::uuid, $2::uuid, 0.01)",
            test_user_id, currency_btc_id
        );

        // Создание тестового пользователя для пустого баланса
        txn.exec_params(
            "INSERT INTO users (id, username, email, password_hash) "
            "VALUES ($1::uuid, $2, $3, $4) ON CONFLICT (id) DO NOTHING",
            empty_user_id, "empty_user_" + empty_user_id, "empty_user@example.com", "empty_password_hash"
        );

        txn.commit(); // Фиксируем все изменения для этой настройки теста
    }

    void TearDown() override {
        pqxx::work txn(*pg_conn); // Начинаем транзакцию для очистки
        txn.exec_params("DELETE FROM accounts WHERE user_id = $1::uuid", test_user_id);
        txn.exec_params("DELETE FROM users WHERE id = $1::uuid", test_user_id);
        txn.exec_params("DELETE FROM users WHERE id = $1::uuid", empty_user_id); // Удаляем пустого пользователя
        txn.exec0("DELETE FROM currencies WHERE code IN ('USD', 'BTC');"); // Убеждаемся в очистке по коду
        txn.commit(); // Фиксируем изменения очистки

        redis_conn.del(test_token);
        redis_conn.del(empty_token); // Удаляем токен для пустого пользователя
        pg_conn.reset(); // Закрываем соединение после каждого теста
    }
};

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
    crow::request req;
    req.body = "{\"token\": \"" + empty_token + "\"}"; // Используем сгенерированный токен
    auto res = handler(req);
    
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
    ASSERT_EQ(json["error"], "System error: Token not found or id field missing");
}

TEST_F(BalanceViewerIntegrationTest, Returns400IfMalformedJson) {
    crow::request req;
    req.body = R"({"token": "test-token"})";

    auto res = handler(req);
    EXPECT_EQ(res.code, 200);

    auto json = nlohmann::json::parse(res.body);
    ASSERT_TRUE(json.contains("balances"));
    EXPECT_DOUBLE_EQ(json["balances"]["USD"], 500.0);
    EXPECT_DOUBLE_EQ(json["balances"]["BTC"], 0.01);
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
