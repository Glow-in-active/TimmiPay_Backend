#pragma once

#include <pqxx/pqxx>
#include "../config/config.h"

/**
 * Устанавливает соединение с базой данных PostgreSQL.
 * 
 * @param config Конфигурация подключения (хост, порт, пользователь, пароль и т.д.).
 * @return pqxx::connection Объект соединения с базой данных.
 * @throws std::runtime_error В случае ошибки подключения или SQL-ошибки.
 */
pqxx::connection connect_to_database(const Config& config);
