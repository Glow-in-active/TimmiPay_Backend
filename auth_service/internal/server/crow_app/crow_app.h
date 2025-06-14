#pragma once

#include <crow.h>
#include <crow/middlewares/cors.h>

#include "../../auth/user_verify_http/session_hold/session_hold.h"
#include "../../auth/user_verify_http/session_start/session_start.h"

/**
 * @brief Создает и настраивает экземпляр crow::App с обработкой CORS.
 *
 * @param session_start_handler Обработчик для начала сессии.
 * @param session_hold_handler Обработчик для удержания сессии (обновления).
 * @return Ссылка на настроенный объект crow::App.
 */
crow::App<crow::CORSHandler>& create_crow_app(
    SessionStart& session_start_handler, SessionHold& session_hold_handler);