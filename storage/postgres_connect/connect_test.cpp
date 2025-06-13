#include <gtest/gtest.h>

#include "../config/config.h"
#include "../postgres_connect/connect.h"

/**
 * @brief Интеграционный тестовый класс для проверки подключения к базе данных PostgreSQL.
 *
 * Инициализирует и настраивает конфигурацию базы данных для каждого теста.
 */
class DatabaseConnectionIntegrationTest : public ::testing::Test {
protected:
    /**
     * @brief Настраивает тестовую среду перед каждым тестом.
     *
     * Загружает валидную конфигурацию для тестового подключения к PostgreSQL.
     */
    void SetUp() override {
        valid_config = load_config("database_config/test_postgres_config.json");
    }

    Config valid_config;
};

/**
 * @brief Проверяет успешное подключение к базе данных.
 *
 * Тест пытается установить соединение с базой данных, используя валидные параметры
 * конфигурации, и проверяет, что соединение открыто и не выбрасывает исключений.
 */
TEST_F(DatabaseConnectionIntegrationTest, ConnectsSuccessfully) {
    EXPECT_NO_THROW({
        auto conn = connect_to_database(valid_config);
        EXPECT_TRUE(conn.is_open());
    });
}

/**
 * @brief Проверяет, что выбрасывается исключение при неверном хосте.
 *
 * Тест пытается подключиться к базе данных с неверным именем хоста и ожидает
 * исключения `std::runtime_error`.
 */
TEST_F(DatabaseConnectionIntegrationTest, ThrowsOnInvalidHost) {
    Config config = valid_config;
    config.host = "invalid_host";

    EXPECT_THROW({
        auto conn = connect_to_database(config);
    }, std::runtime_error);
}

/**
 * @brief Проверяет, что выбрасывается исключение при неверном порте.
 *
 * Тест пытается подключиться к базе данных с неверным номером порта и ожидает
 * исключения `std::runtime_error`.
 */
TEST_F(DatabaseConnectionIntegrationTest, ThrowsOnInvalidPort) {
    Config config = valid_config;
    config.port = 9999;

    EXPECT_THROW({
        auto conn = connect_to_database(config);
    }, std::runtime_error);
}

/**
 * @brief Проверяет, что выбрасывается исключение при неверных учетных данных.
 *
 * Тест пытается подключиться к базе данных с неверными именем пользователя и/или
 * паролем и ожидает исключения `std::runtime_error`.
 */
TEST_F(DatabaseConnectionIntegrationTest, ThrowsOnInvalidCredentials) {
    Config config = valid_config;
    config.user = "wrong_user";
    config.password = "wrong_password";

    EXPECT_THROW({
        auto conn = connect_to_database(config);
    }, std::runtime_error);
}

/**
 * @brief Проверяет, что выбрасывается исключение при неверном имени базы данных.
 *
 * Тест пытается подключиться к несуществующей базе данных и ожидает
 * исключения `std::runtime_error`.
 */
TEST_F(DatabaseConnectionIntegrationTest, ThrowsOnInvalidDatabaseName) {
    Config config = valid_config;
    config.dbname = "nonexistent_db";

    EXPECT_THROW({
        auto conn = connect_to_database(config);
    }, std::runtime_error);
}
