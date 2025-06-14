#pragma once

#include <crow.h>
#include <crow/middlewares/cors.h>

#include "../../auth/user_verify_http/session_hold/session_hold.h"
#include "../../auth/user_verify_http/session_start/session_start.h"

/**
 * @brief Регистрирует маршруты для приложения crow.
 *
 * @param app Ссылка на объект crow::App, к которому будут добавлены маршруты.
 * @param session_start_handler Обработчик для начала сессии.
 * @param session_hold_handler Обработчик для удержания сессии (обновления).
 */
void register_routes(crow::App<crow::CORSHandler>& app,
                     SessionStart& session_start_handler,
                     SessionHold& session_hold_handler);