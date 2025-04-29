#include "../internal/storage/config/config.h"
#include "../internal/storage/postgres_connect/connect.h"
#include "../internal/storage/user_verify/auth/user_verify.h"
#include "../internal/user_verify/verification/user_verify.h"
#include "../internal/storage/redis_config/config_redis.h"
#include "../internal/storage/redis_connect/connect_redis.h"
#include "../internal/user_verify_http/session_start/session_start.h"
#include <crow.h>
#include <crow/middlewares/cors.h>
#include <pqxx/pqxx>
#include <sw/redis++/redis++.h>

using json = nlohmann::json;

int main() {
    try {
        // Инициализация конфигов и соединений
        Config postgres_config = load_config("postgres-config.json");
        ConfigRedis redis_config = load_redis_config("redis-config.json");
        
        pqxx::connection postgres_conn = connect_to_database(postgres_config);
        sw::redis::Redis redis_conn = connect_to_redis(redis_config);

        // Создаем цепочку зависимостей
        UserVerifier user_verifier(postgres_conn, redis_conn);
        SessionStart session_handler(user_verifier);  // Наш модуль

        // Настройка сервера Crow
        crow::App<crow::CORSHandler> app;
        
        // Конфигурация CORS
        auto& cors = app.get_middleware<crow::CORSHandler>();
        cors
          .global()
            .headers("Content-Type")
            .methods("POST"_method)
            .origin("*");

        // Обработчик эндпоинта
        CROW_ROUTE(app, "/session_start").methods("POST"_method)(
            [&session_handler](const crow::request& req) {
                try {
                    // Парсим напрямую через nlohmann::json
                    json request_body = json::parse(req.body);
                    
                    // Делегируем обработку в SessionStart
                    json response = session_handler.HandleRequest(request_body);
                    
                    // Формируем HTTP-ответ
                    if (response.contains("error")) {
                        const int status_code = 
                            (response["error"] == "Invalid JSON format") ? 400 :
                            (response["error"] == "Verification failed") ? 401 : 500;
                        return crow::response(status_code, response.dump());
                    }
                    return crow::response(200, response.dump());
                    
                } catch (const std::exception& e) {
                    return crow::response(500, json{{"error", e.what()}}.dump());
                }
            });

        // Запуск сервера
        app.port(8080).multithreaded().run();
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}