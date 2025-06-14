#include "db_init.h"

#include <gtest/gtest.h>

#include "../../../storage/config/config.h"
#include "../../../storage/postgres_connect/connect.h"
#include "../../../storage/redis_config/config_redis.h"
#include "../../../storage/redis_connect/connect_redis.h"

/**
 * @brief Инициализирует тестовые соединения с базами данных PostgreSQL и Redis.
 *
 * Загружает тестовые конфигурации для PostgreSQL и Redis, устанавливает
 * соединения и возвращает их.
 *
 * @return Структура DBConnections, содержащая установленные тестовые соединения
 * с PostgreSQL и Redis.
 * @throws std::runtime_error Если инициализация тестовой базы данных
 * завершается с ошибкой.
 */
DBConnections initialize_auth_test_databases() {
  try {
    Config postgres_config =
        load_config("database_config/test_postgres_config.json");
    ConfigRedis redis_config =
        load_redis_config("database_config/test_redis_config.json");

    pqxx::connection postgres_conn = connect_to_database(postgres_config);
    sw::redis::Redis redis_conn = connect_to_redis(redis_config);

    return {std::move(postgres_conn), std::move(redis_conn)};

  } catch (const std::exception& e) {
    throw std::runtime_error("Test database initialization failed: " +
                             std::string(e.what()));
  }
}

/**
 * @brief Проверяет, что соединение с PostgreSQL открыто.
 *
 * Тест инициализирует тестовые соединения с базами данных и проверяет, что
 * соединение с PostgreSQL активно.
 */
TEST(DBInitTest, TestPostgresConnectionIsOpen) {
  DBConnections db = initialize_auth_test_databases();
  EXPECT_TRUE(db.postgres.is_open());
}

/**
 * @brief Проверяет, что соединение с Redis активно.
 *
 * Тест инициализирует тестовые соединения с базами данных и отправляет команду
 * PING в Redis для проверки активности соединения.
 */
TEST(DBInitTest, TestRedisConnectionIsAlive) {
  DBConnections db = initialize_auth_test_databases();
  EXPECT_NO_THROW({
    std::string pong = db.redis.ping();
    EXPECT_EQ(pong, "PONG");
  });
}
