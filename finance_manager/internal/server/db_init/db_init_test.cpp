#include "db_init.h"

#include <gtest/gtest.h>

#include "../../../../storage/config/config.h"
#include "../../../../storage/postgres_connect/connect.h"
#include "../../../../storage/redis_config/config_redis.h"
#include "../../../../storage/redis_connect/connect_redis.h"

/**
 * @brief Инициализирует тестовые соединения с базами данных PostgreSQL и Redis
 * для финансовых тестов.
 *
 * Загружает конфигурации для тестовых баз данных, устанавливает соединения и
 * возвращает их.
 *
 * @return Структура DBConnections, содержащая установленные тестовые
 * соединения.
 * @throws std::runtime_error Если инициализация тестовой базы данных
 * завершается с ошибкой.
 */
DBConnections initialize_finance_test_databases() {
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
 * @brief Проверяет, что соединение с PostgreSQL успешно открыто.
 *
 * Тест инициализирует тестовые базы данных и проверяет, что соединение с
 * PostgreSQL находится в открытом состоянии.
 */
TEST(FinanceDBInitTest, TestPostgresConnectionIsOpen) {
  DBConnections db = initialize_finance_test_databases();
  EXPECT_TRUE(db.postgres.is_open());
}

/**
 * @brief Проверяет, что соединение с Redis активно.
 *
 * Тест инициализирует тестовые базы данных и проверяет, что соединение с Redis
 * активно, выполняя команду PING.
 */
TEST(FinanceDBInitTest, TestRedisConnectionIsAlive) {
  DBConnections db = initialize_finance_test_databases();
  EXPECT_NO_THROW({
    std::string pong = db.redis.ping();
    EXPECT_EQ(pong, "PONG");
  });
}

/**
 * @brief Проверяет обработку некорректной конфигурации PostgreSQL.
 *
 * Тест пытается подключиться к PostgreSQL с невалидной конфигурацией
 * и ожидает возникновения `std::runtime_error`.
 */
TEST(FinanceDBInitTest, TestInvalidPostgresConfig) {
  Config invalid_config;
  invalid_config.host = "invalid_host";
  invalid_config.port = 12345;

  EXPECT_THROW(
      { pqxx::connection conn = connect_to_database(invalid_config); },
      std::runtime_error);
}

/**
 * @brief Проверяет обработку некорректной конфигурации Redis.
 *
 * Тест пытается подключиться к Redis с невалидной конфигурацией
 * и ожидает возникновения `std::runtime_error`.
 */
TEST(FinanceDBInitTest, TestInvalidRedisConfig) {
  ConfigRedis invalid_config;
  invalid_config.host = "invalid_host";
  invalid_config.port = 12345;

  EXPECT_THROW(
      { sw::redis::Redis conn = connect_to_redis(invalid_config); },
      std::runtime_error);
}