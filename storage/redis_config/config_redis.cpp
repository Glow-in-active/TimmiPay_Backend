#include "config_redis.h"
#include <fstream>
#include <nlohmann/json.hpp>

/**
 * @brief Загружает конфигурацию Redis из JSON-файла.
 *
 * Открывает файл по указанному пути, парсит JSON и заполняет структуру ConfigRedis.
 *
 * @param filename Путь к JSON-файлу с конфигурацией Redis.
 * @return Структура ConfigRedis с параметрами конфигурации Redis.
 * @throws std::runtime_error Если файл не удалось открыть.
 * @throws nlohmann::json::parse_error Если JSON некорректен.
 */
ConfigRedis load_redis_config(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open Redis config file: " + filename);
    }

    nlohmann::json data = nlohmann::json::parse(file);

    return ConfigRedis {
        .host = data["host"].get<std::string>(),
        .port = data["port"].get<int>(),
        .password = data["password"].get<std::string>(),
        .db = data["db"].get<int>()
    };
}