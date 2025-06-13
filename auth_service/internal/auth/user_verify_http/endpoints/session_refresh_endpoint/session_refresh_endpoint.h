#pragma once

#include <crow.h>
#include "../../session_hold/session_hold.h"

/**
 * @brief Создает обработчик HTTP-запросов для обновления сессии.
 *
 * @param handler Объект SessionHold, который обрабатывает логику удержания/обновления сессии.
 * @return Функция, которая принимает `crow::request` и возвращает `crow::response`.
 */
std::function<crow::response(const crow::request&)> create_session_refresh_handler(SessionHold& handler);