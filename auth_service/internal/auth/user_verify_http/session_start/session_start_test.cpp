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

/**
 * @brief Тестовый класс для интеграционных тестов SessionStart.
 *
 * Настраивает и очищает тестовую среду с использованием реальных подключений к PostgreSQL и Redis,
 * а также тестовых пользователей.
 */
class ProdSessionTest : public ::testing::Test {
protected:
    pqxx::connection pg_conn;
    sw::redis::Redis redis_conn;
    UserVerifier verifier;

    std::string test_email = "test_user@example.com";
    std::string test_hash = "5f4dcc3b5aa765d61d8327deb882cf99";
    std::string test_username = "aboba";

    /**
     * @brief Конструктор класса ProdSessionTest.
     *
     * Инициализирует соединения с тестовыми базами данных PostgreSQL и Redis,
     * а также UserVerifier.
     */
    ProdSessionTest()
        : pg_conn(connect_to_database(load_config("database_config/test_postgres_config.json"))),
          redis_conn(connect_to_redis(load_redis_config("database_config/test_redis_config.json"))),
          verifier(pg_conn, redis_conn) {}

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
     * Удаляет тестового пользователя из базы данных PostgreSQL.
     */
    void TearDown() override {
        pqxx::work txn(pg_conn);
        txn.exec_params("DELETE FROM users WHERE email = $1", test_email);
        txn.commit();
    }
};

/**
 * @brief Проверяет интеграцию с реальной базой данных для начала сессии.
 *
 * Тест использует реальные соединения с базами данных для начала сессии
 * и проверяет, что токен успешно генерируется и нет ошибок.
 */
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
