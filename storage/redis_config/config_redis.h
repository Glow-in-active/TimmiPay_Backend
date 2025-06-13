#pragma once

#include <string>

/**
 * @brief Структура для хранения параметров конфигурации подключения к Redis.
 */
struct ConfigRedis {
    std::string host;
    int port;
    std::string password;
    int db;
};

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
ConfigRedis load_redis_config(const std::string& filename);