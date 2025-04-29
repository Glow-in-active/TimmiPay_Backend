#include "../internal/storage/config/config.h"
#include "../internal/storage/postgres_connect/connect.h"
#include "../internal/storage/user_verify/auth/user_verify.h"
#include "../internal/user_verify/verification/user_verify.h"
#include "../internal/storage/redis_config/config_redis.h"
#include "../internal/storage/redis_connect/connect_redis.h"
#include "../internal/user_verify_http/session_start/session_start.h"
#include "../internal/user_verify_http/session_hold/session_hold.h"
#include <crow.h>
#include <crow/middlewares/cors.h>
#include <pqxx/pqxx>
#include <sw/redis++/redis++.h>

using json = nlohmann::json;

int main() {
    try {
        // Инициализация конфигов
        Config postgres_config = load_config("postgres-config.json");
        ConfigRedis redis_config = load_redis_config("redis-config.json");
        
        // Установка соединений
        pqxx::connection postgres_conn = connect_to_database(postgres_config);
        sw::redis::Redis redis_conn = connect_to_redis(redis_config);

        // Инициализация зависимостей
        UserVerifier user_verifier(postgres_conn, redis_conn);
        SessionStart session_start_handler(user_verifier);
        SessionHold session_hold_handler(redis_conn);

        // Настройка Crow приложения
        crow::App<crow::CORSHandler> app;
        
        // Конфигурация CORS
        auto& cors = app.get_middleware<crow::CORSHandler>();
        cors
          .global()
            .headers("Content-Type", "Authorization")
            .methods("POST"_method)
            .origin("*");

        // Эндпоинт старта сессии
        CROW_ROUTE(app, "/session_start").methods("POST"_method)(
            [&session_start_handler](const crow::request& req) {
                try {
                    json request_body = json::parse(req.body);
                    json response = session_start_handler.HandleRequest(request_body);
                    
                    if (response.contains("error")) {
                        int status_code = 500;
                        if (response["error"] == "Invalid JSON format") status_code = 400;
                        else if (response["error"] == "Verification failed") status_code = 401;
                        
                        return crow::response(status_code, response.dump());
                    }
                    return crow::response(200, response.dump());
                    
                } catch (const std::exception& e) {
                    return crow::response(500, json{{"error", "Internal server error"}}.dump());
                }
            });

        // Эндпоинт продления сессии
        CROW_ROUTE(app, "/session_refresh").methods("POST"_method)(
            [&session_hold_handler](const crow::request& req) {
                try {
                    json request_body = json::parse(req.body);
                    json response = session_hold_handler.HandleRequest(request_body);
                    
                    if (response.contains("error")) {
                        int status_code = 500;
                        if (response["error"] == "Invalid JSON format") status_code = 400;
                        else if (response["error"] == "Token not found or expired") status_code = 404;
                        
                        return crow::response(status_code, response.dump());
                    }
                    return crow::response(200, response.dump());
                    
                } catch (const json::exception& e) {
                    return crow::response(400, json{{"error", "Invalid JSON format"}}.dump());
                } catch (const std::exception& e) {
                    return crow::response(500, json{{"error", "Internal server error"}}.dump());
                }
            });

        // Запуск сервера
        app.port(8080)
            .multithreaded()
            .run();

    } catch (const std::exception& e) {
        std::cerr << "Fatal initialization error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}