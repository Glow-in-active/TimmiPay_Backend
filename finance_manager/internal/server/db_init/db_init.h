#pragma once

#include "../../../storage/config/config.h"
#include "../../../storage/redis_config/config_redis.h"
#include <pqxx/pqxx>
#include <sw/redis++/redis++.h>

/**
 * @brief Структура для хранения соединений с базами данных PostgreSQL и Redis.
 */
struct DBConnections {
    pqxx::connection postgres;
    sw::redis::Redis redis;
};

/**
 * @brief Инициализирует соединения с базами данных PostgreSQL и Redis.
 *
 * @return Структура DBConnections, содержащая установленные соединения с PostgreSQL и Redis.
 */
DBConnections initialize_databases(); 