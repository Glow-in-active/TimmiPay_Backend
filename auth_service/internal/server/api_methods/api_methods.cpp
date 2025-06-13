#include "api_methods.h"
#include "../../auth/user_verify_http/endpoints/session_auth_endpoint/session_auth_endpoint.h"
#include "../../auth/user_verify_http/endpoints/session_refresh_endpoint/session_refresh_endpoint.h"

/**
 * @brief Регистрирует маршруты для приложения crow.
 *
 * Эта функция настраивает маршруты для аутентификации сессий и обновления сессий.
 *
 * @param app Ссылка на объект crow::App, к которому будут добавлены маршруты.
 * @param session_start_handler Обработчик для начала сессии.
 * @param session_hold_handler Обработчик для удержания сессии (обновления).
 */
void register_routes(crow::App<crow::CORSHandler>& app, SessionStart& session_start_handler, SessionHold& session_hold_handler) {
    CROW_ROUTE(app, "/session_start").methods("POST"_method)(
        create_session_auth_handler(session_start_handler)
    );

    CROW_ROUTE(app, "/session_refresh").methods("POST"_method)(
        create_session_refresh_handler(session_hold_handler)
    );
}




