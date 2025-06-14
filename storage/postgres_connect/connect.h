#pragma once

#include <pqxx/pqxx>

#include "../config/config.h"

/**
 * @brief Устанавливает соединение с базой данных PostgreSQL.
 *
 * @param config Объект Config, содержащий параметры подключения к базе данных.
 * @return Активное соединение с базой данных PostgreSQL (`pqxx::connection`).
 * @throws std::runtime_error В случае ошибок подключения к базе данных или
 * SQL-ошибок.
 */
pqxx::connection connect_to_database(const Config& config);
