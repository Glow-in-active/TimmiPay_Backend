#include "config.h"

#include <fstream>
#include <nlohmann/json.hpp>

/**
 * @brief Загружает конфигурацию из JSON-файла.
 *
 * Открывает файл по указанному пути, парсит JSON и заполняет структуру Config.
 *
 * @param filename Путь к JSON-файлу с конфигурацией.
 * @return Структура Config с параметрами конфигурации.
 * @throws std::runtime_error Если файл не удалось открыть.
 * @throws nlohmann::json::parse_error Если JSON некорректен.
 */
Config load_config(const std::string& filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open config file: " + filename);
  }

  nlohmann::json data = nlohmann::json::parse(file);

  return Config{.host = data["host"].get<std::string>(),
                .port = data["port"].get<int>(),
                .user = data["user"].get<std::string>(),
                .password = data["password"].get<std::string>(),
                .dbname = data["dbname"].get<std::string>(),
                .sslmode = data["sslmode"].get<std::string>()};
}
