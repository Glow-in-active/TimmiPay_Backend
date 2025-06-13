#include <gtest/gtest.h>
#include <pqxx/pqxx>
#include <sw/redis++/redis.h>
#include <nlohmann/json.hpp>
#include <iterator>

#include "../../user_verify/verification/user_verify.h"
#include "session_hold.h"
#include "../session_start/session_start.h"

#include "../../../../storage/config/config.h"
#include "../../../../storage/redis_config/config_redis.h"
#include "../../../../storage/postgres_connect/connect.h"
#include "../../../../storage/redis_connect/connect_redis.h"

const std::string REDIS_PREFIX = "session:";

/**
 * @brief Тестовый класс для интеграционных тестов SessionHold.
 *
 * Настраивает и очищает тестовую среду с использованием реальных подключений к PostgreSQL и Redis,
 * а также тестовых пользователей.
 */
class ProdSessionHoldTest : public ::testing::Test {
protected:
    Config pg_config = load_config("database_config/test_postgres_config.json");
    ConfigRedis redis_config = load_redis_config("database_config/test_redis_config.json");

    pqxx::connection pg_conn = connect_to_database(pg_config);
    sw::redis::Redis redis_conn = connect_to_redis(redis_config);

    UserVerifier verifier{pg_conn, redis_conn};

    std::string test_email = "test_user@example.com";
    std::string test_hash = "5f4dcc3b5aa765d61d8327deb882cf99";
    std::string test_username = "aboba";

    /**
     * @brief Настраивает тестовую среду перед каждым тестом.
     *
     * Вставляет тестового пользователя в базу данных PostgreSQL.
     */
    void SetUp() override {
        pqxx::work txn(pg_conn);
        txn.exec_params(
            "INSERT INTO users (email, password_hash, username) VALUES ($1, $2, $3) "
            "ON CONFLICT (email) DO NOTHING",
            test_email, test_hash, test_username);
        txn.commit();
    }

    /**
     * @brief Очищает тестовую среду после каждого теста.
     *
     * Удаляет тестового пользователя из PostgreSQL и все связанные ключи сессий из Redis.
     */
    void TearDown() override {
        pqxx::work txn(pg_conn);
        txn.exec_params("DELETE FROM users WHERE email = $1", test_email);
        txn.commit();

        std::vector<std::string> keys;
        redis_conn.keys(REDIS_PREFIX + "*", std::back_inserter(keys));
        for (const auto& key : keys) {
            redis_conn.del(key);
        }
    }
};

/**
 * @brief Проверяет успешное удержание (обновление) сессии с валидным токеном.
 *
 * Тест генерирует валидный токен, использует его для удержания сессии
 * и проверяет, что операция успешна, а токен по-прежнему существует в Redis.
 */
TEST_F(ProdSessionHoldTest, SessionHoldValidToken) {
    SessionStart session_handler(verifier);
    nlohmann::json session_request = {
        {"email", test_email},
        {"password_hash", test_hash}
    };
    nlohmann::json session_response = session_handler.HandleRequest(session_request);
    std::string token = session_response["token"];

    SessionHold hold_handler(redis_conn);
    nlohmann::json hold_request = {{"token", token}};
    nlohmann::json hold_response = hold_handler.HandleRequest(hold_request);

    EXPECT_EQ(hold_response["status"], "success");
    EXPECT_FALSE(hold_response.contains("error"));

    bool token_exists = redis_conn.exists(token);
    EXPECT_TRUE(token_exists) << "Ожидаемый ключ: " << token;
}

/**
 * @brief Проверяет обработку невалидного или несуществующего токена при удержании сессии.
 *
 * Тест пытается удержать сессию с невалидным токеном и проверяет,
 * что возвращается ошибка "Token not found or expired".
 */
TEST_F(ProdSessionHoldTest, SessionHoldInvalidToken) {
    SessionHold hold_handler(redis_conn);

    nlohmann::json request = {{"token", "invalid_nonexistent_token"}};
    nlohmann::json response = hold_handler.HandleRequest(request);

    EXPECT_TRUE(response.contains("error"));
    EXPECT_EQ(response["error"], "Token not found or expired");
}

/**
 * @brief Проверяет интеграцию с реальной базой данных для начала сессии.
 *
 * Тест использует реальные соединения с базами данных для начала сессии
 * и проверяет, что токен успешно генерируется и нет ошибок.
 */
TEST_F(ProdSessionHoldTest, RealDbIntegration) {
    SessionStart session_handler(verifier);

    nlohmann::json request = {
        {"email", test_email},
        {"password_hash", test_hash}
    };

    nlohmann::json response = session_handler.HandleRequest(request);

    EXPECT_FALSE(response["token"].empty());
    EXPECT_FALSE(response.contains("error"));
}
