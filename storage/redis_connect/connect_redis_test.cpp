#include <gtest/gtest.h>
#include <sw/redis++/redis++.h>
#include "../config/config.h"
#include "../redis_config/config_redis.h"
#include "../postgres_connect/connect.h"
#include "../redis_connect/connect_redis.h"

/**
 * @brief Тестовый класс для проверки подключения к Redis.
 *
 * Инициализирует конфигурацию Redis для каждого теста.
 */
class ConnectRedisTest : public ::testing::Test {
protected:
    /**
     * @brief Настраивает тестовую среду перед каждым тестом.
     *
     * Загружает валидную конфигурацию Redis для тестового подключения.
     */
    void SetUp() override {
        redis_config = load_redis_config("database_config/test_redis_config.json");
    }

    ConfigRedis redis_config;
};

/**
 * @brief Проверяет успешное подключение к Redis с валидной конфигурацией.
 *
 * Тест пытается установить соединение с Redis, используя валидные параметры
 * конфигурации, и проверяет, что соединение установлено и не выбрасывает исключений.
 */
TEST_F(ConnectRedisTest, ConnectsWithValidConfig) {
    EXPECT_NO_THROW({
        auto redis = connect_to_redis(redis_config);
        redis.ping();
    });
}
