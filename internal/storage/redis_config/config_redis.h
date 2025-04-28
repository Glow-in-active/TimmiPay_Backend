#pragma once

#include <string>

/**
 * @brief Структура для хранения параметров подключения к Redis
 */
struct ConfigRedis {
    std::string host;
    int port;
    std::string password;
    int db;
};

/**
 * @brief Загружает конфигурацию Redis из JSON-файла
 * 
 * @param filename Путь к JSON-файлу с конфигурацией
 * @return ConfigRedis Структура с параметрами подключения
 * @throws std::runtime_error Если файл не найден или произошла ошибка парсинга
 * 
 * Пример JSON-файла:
 * @code{.json}
 * {
 *   "host": "localhost",
 *   "port": 6379,
 *   "password": "",
 *   "db": 0
 * }
 * @endcode
 */
ConfigRedis load_redis_config(const std::string& filename);