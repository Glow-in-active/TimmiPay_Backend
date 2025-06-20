#include "db_init.h"

#include "../../../storage/config/config.h"
#include "../../../storage/postgres_connect/connect.h"
#include "../../../storage/redis_config/config_redis.h"
#include "../../../storage/redis_connect/connect_redis.h"

/**
 * @brief Инициализирует соединения с базами данных PostgreSQL и Redis.
 *
 * Загружает конфигурации для PostgreSQL и Redis, устанавливает соединения и
 * возвращает их.
 *
 * @return Структура DBConnections, содержащая установленные соединения с
 * PostgreSQL и Redis.
 * @throws std::runtime_error Если инициализация базы данных завершается с
 * ошибкой.
 */
DBConnections initialize_databases() {
  try {
    Config postgres_config =
        load_config("database_config/prod_postgres_config.json");
    ConfigRedis redis_config =
        load_redis_config("database_config/prod_redis_config.json");

    pqxx::connection postgres_conn = connect_to_database(postgres_config);
    sw::redis::Redis redis_conn = connect_to_redis(redis_config);

    return {std::move(postgres_conn), std::move(redis_conn)};

  } catch (const std::exception& e) {
    throw std::runtime_error("Database initialization failed: " +
                             std::string(e.what()));
  }
}