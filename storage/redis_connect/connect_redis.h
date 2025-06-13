#pragma once

#include <sw/redis++/redis++.h>
#include "../redis_config/config_redis.h"

/**
 * @brief Устанавливает соединение с сервером Redis.
 *
 * Создает параметры подключения на основе предоставленной конфигурации и пытается
 * установить соединение с Redis.
 *
 * @param config Объект ConfigRedis, содержащий параметры подключения к Redis.
 * @return Активное соединение с Redis (`sw::redis::Redis`).
 * @throws std::runtime_error В случае ошибок подключения к Redis или системных ошибок,
 *                             или если индекс базы данных Redis находится вне допустимого диапазона (0-15).
 */
sw::redis::Redis connect_to_redis(const ConfigRedis& config);
