#pragma once

#include <sw/redis++/redis++.h>
#include "../redis_config/config_redis.h"

/**
 * Устанавливает соединение с Redis с использованием полной конфигурации.
 * 
 * @param config Конфигурация подключения (host, port, password, db)
 * @return sw::redis::Redis Готовый к работе клиент Redis
 * @throws std::runtime_error При ошибках подключения или аутентификации
 */
sw::redis::Redis connect_to_redis(const ConfigRedis& config);
