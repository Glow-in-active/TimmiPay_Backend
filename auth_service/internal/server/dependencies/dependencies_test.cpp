#include "dependencies.h"

#include <crow.h>
#include <gtest/gtest.h>

#include "../../../storage/config/config.h"
#include "../../../storage/postgres_connect/connect.h"
#include "../../../storage/redis_config/config_redis.h"
#include "../../../storage/redis_connect/connect_redis.h"
#include "../../auth/user_verify_http/endpoints/session_auth_endpoint/session_auth_endpoint.h"
#include "../../auth/user_verify_http/endpoints/session_refresh_endpoint/session_refresh_endpoint.h"
#include "../db_init/db_init.h"

/**
 * @brief Инициализирует тестовые соединения с базами данных PostgreSQL и Redis.
 *
 * Загружает конфигурации для тестовых баз данных, устанавливает соединения и
 * возвращает их.
 *
 * @return Структура DBConnections, содержащая установленные тестовые
 * соединения.
 */
static DBConnections initialize_test_databases_for_dependencies() {
  Config postgres_config =
      load_config("database_config/test_postgres_config.json");
  ConfigRedis redis_config =
      load_redis_config("database_config/test_redis_config.json");

  pqxx::connection postgres_conn = connect_to_database(postgres_config);
  sw::redis::Redis redis_conn = connect_to_redis(redis_config);

  return {std::move(postgres_conn), std::move(redis_conn)};
}

/**
 * @brief Проверяет, что инициализация зависимостей не вызывает исключений.
 *
 * Тест создает тестовые соединения с базами данных и пытается инициализировать
 * зависимости, ожидая, что процесс завершится без ошибок.
 */
TEST(DependenciesTest, InitializeDependenciesDoesNotThrow) {
  DBConnections db = initialize_test_databases_for_dependencies();
  EXPECT_NO_THROW({ Dependencies deps = initialize_dependencies(db); });
}

/**
 * @brief Проверяет, что инициализированные обработчики являются валидными.
 *
 * Тест инициализирует зависимости и проверяет, что указатели на
 * `user_verifier`, `session_start_handler` и `session_hold_handler` не являются
 * nullptr, подтверждая их успешное создание.
 */
TEST(DependenciesTest, HandlersAreValid) {
  DBConnections db = initialize_test_databases_for_dependencies();
  Dependencies deps = initialize_dependencies(db);

  EXPECT_TRUE(&deps.user_verifier != nullptr);
  EXPECT_TRUE(&deps.session_start_handler != nullptr);
  EXPECT_TRUE(&deps.session_hold_handler != nullptr);
}