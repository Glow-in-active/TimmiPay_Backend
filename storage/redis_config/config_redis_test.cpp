#include "config_redis.h"

#include <gtest/gtest.h>

#include <fstream>
#include <nlohmann/json.hpp>

/**
 * @brief Проверяет корректную загрузку конфигурации Redis из файла.
 *
 * Тест создает временный JSON-файл с валидными данными конфигурации Redis,
 * загружает его с помощью функции `load_redis_config` и проверяет, что все поля
 * структуры `ConfigRedis` заполнены правильно.
 */
TEST(RedisConfigTest, LoadsCorrectConfig) {
  const std::string filename = "test_redis_config.json";
  {
    std::ofstream file(filename);
    file << R"({
            "host": "redis.example.com",
            "port": 6379,
            "password": "secret123",
            "db": 5
        })";
  }

  ConfigRedis config = load_redis_config(filename);

  EXPECT_EQ(config.host, "redis.example.com");
  EXPECT_EQ(config.port, 6379);
  EXPECT_EQ(config.password, "secret123");
  EXPECT_EQ(config.db, 5);

  std::remove(filename.c_str());
}

/**
 * @brief Проверяет, что функция выбрасывает исключение при отсутствии файла
 * конфигурации Redis.
 *
 * Тест вызывает `load_redis_config` с именем несуществующего файла и ожидает
 * исключения `std::runtime_error`.
 */
TEST(RedisConfigTest, ThrowsOnMissingFile) {
  EXPECT_THROW(
      { load_redis_config("non_existent_redis_config.json"); },
      std::runtime_error);
}

/**
 * @brief Проверяет, что функция выбрасывает исключение при отсутствии
 * обязательного поля в конфигурации Redis.
 *
 * Тест создает JSON-файл, в котором отсутствует одно из обязательных полей
 * конфигурации Redis, вызывает `load_redis_config` и ожидает исключения
 * `nlohmann::json::exception`.
 */
TEST(RedisConfigTest, ThrowsOnMissingField) {
  const std::string filename = "missing_redis_field.json";
  {
    std::ofstream file(filename);
    file << R"({
            "host": "localhost",
            "port": 6379,
            "password": ""
        })";
  }

  EXPECT_THROW({ load_redis_config(filename); }, nlohmann::json::exception);

  std::remove(filename.c_str());
}

/**
 * @brief Проверяет, что функция выбрасывает исключение при неверном типе данных
 * поля в конфигурации Redis.
 *
 * Тест создает JSON-файл, где значение поля `port` имеет неверный тип (строка
 * вместо числа), вызывает `load_redis_config` и ожидает исключения
 * `nlohmann::json::exception`.
 */
TEST(RedisConfigTest, ThrowsOnInvalidType) {
  const std::string filename = "invalid_redis_type.json";
  {
    std::ofstream file(filename);
    file << R"({
            "host": "localhost",
            "port": "6379",
            "password": "",
            "db": 0
        })";
  }

  EXPECT_THROW({ load_redis_config(filename); }, nlohmann::json::exception);

  std::remove(filename.c_str());
}

/**
 * @brief Проверяет, что функция выбрасывает исключение при некорректном формате
 * JSON в конфигурации Redis.
 *
 * Тест создает файл с некорректным JSON-форматом, вызывает `load_redis_config`
 * и ожидает исключения `nlohmann::json::parse_error`.
 */
TEST(RedisConfigTest, ThrowsOnMalformedJSON) {
  const std::string filename = "malformed_redis.json";
  {
    std::ofstream file(filename);
    file << "{ invalid_json }";
  }

  EXPECT_THROW({ load_redis_config(filename); }, nlohmann::json::parse_error);

  std::remove(filename.c_str());
}

/**
 * @brief Проверяет, что функция выбрасывает исключение при пустом JSON-файле
 * конфигурации Redis.
 *
 * Тест создает пустой файл, вызывает `load_redis_config` и ожидает исключения
 * `nlohmann::json::parse_error`.
 */
TEST(RedisConfigTest, ThrowsOnEmptyJSON) {
  const std::string filename = "empty_redis.json";
  {
    std::ofstream file(filename);
    file << "";
  }

  EXPECT_THROW({ load_redis_config(filename); }, nlohmann::json::parse_error);

  std::remove(filename.c_str());
}