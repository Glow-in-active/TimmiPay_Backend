#pragma once

#include <crow.h>

#include "../../session_start/session_start.h"

/**
 * @brief Создает обработчик HTTP-запросов для аутентификации сессии.
 *
 * @param handler Объект SessionStart, который обрабатывает логику начала
 * сессии.
 * @return Функция, которая принимает `crow::request` и возвращает
 * `crow::response`.
 */
std::function<crow::response(const crow::request&)> create_session_auth_handler(
    SessionStart& handler);