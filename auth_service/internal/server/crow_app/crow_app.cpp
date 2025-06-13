#include "crow_app.h"
#include "../api_methods/api_methods.h"

/**
 * @brief Создает и настраивает экземпляр crow::App с обработкой CORS.
 *
 * Эта функция инициализирует приложение Crow, настраивает глобальные заголовки CORS, методы и источники,
 * а также регистрирует маршруты API.
 *
 * @param session_start_handler Обработчик для начала сессии.
 * @param session_hold_handler Обработчик для удержания сессии (обновления).
 * @return Ссылка на настроенный объект crow::App.
 */
crow::App<crow::CORSHandler>& create_crow_app(
    SessionStart& session_start_handler,
    SessionHold& session_hold_handler
) {
    static crow::App<crow::CORSHandler> app;
    
    auto& cors = app.get_middleware<crow::CORSHandler>();
    cors.global()
        .headers("Content-Type", "Authorization")
        .methods("POST"_method)
        .origin("*");

    register_routes(app, session_start_handler, session_hold_handler);
    
    return app;
}