#pragma once

#include <string>

/**
 * @brief Структура для хранения параметров конфигурации подключения к базе
 * данных PostgreSQL.
 */
struct Config {
  std::string host;
  int port;
  std::string user;
  std::string password;
  std::string dbname;
  std::string sslmode;
};

/**
 * @brief Загружает конфигурацию из JSON-файла.
 *
 * Открывает файл по указанному пути, парсит JSON и заполняет структуру Config.
 *
 * @param filename Путь к JSON-файлу с конфигурацией.
 * @return Структура Config с параметрами конфигурации.
 * @throws std::runtime_error Если файл не удалось открыть или произошла ошибка
 * при парсинге.
 *
 * Пример JSON-файла:
 * @code{.json}
 * {
 *   "host": "localhost",
 *   "port": 5432,
 *   "user": "admin",
 *   "password": "secret",
 *   "dbname": "mydb",
 *   "sslmode": "require"
 * }
 * @endcode
 */
Config load_config(const std::string& filename);
