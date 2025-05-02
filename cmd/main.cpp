#include "session_auth_endpoint.h"
#include "session_refresh_endpoint.h"
#include "../internal/storage/config/config.h"
#include "../internal/storage/postgres_connect/connect.h"
#include "../internal/storage/user_verify/auth/user_verify.h"
#include "../internal/auth/user_verify/verification/user_verify.h"
#include "../internal/storage/redis_config/config_redis.h"
#include "../internal/storage/redis_connect/connect_redis.h"
#include "../internal/auth/user_verify_http/session_start/session_start.h"
#include "../internal/auth/user_verify_http/session_hold/session_hold.h"
#include <crow.h>
#include <crow/middlewares/cors.h>
#include <pqxx/pqxx>
#include <sw/redis++/redis++.h>
#include <iostream>

int main() {
    try {
        // Инициализация конфигов
        Config postgres_config = load_config("database_config/prod_postgres_config.json");
        ConfigRedis redis_config = load_redis_config("database_config/prod_redis_config.json");
        
        // Установка соединений с БД
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

        // Регистрация эндпоинтов
        CROW_ROUTE(app, "/session_start").methods("POST"_method)(
            create_session_auth_handler(session_start_handler)
        );

        CROW_ROUTE(app, "/session_refresh").methods("POST"_method)(
            create_session_refresh_handler(session_hold_handler)
        );

        // Запуск сервера
        app.port(8080)
            .multithreaded()
            .run();

    } catch (const pqxx::sql_error& e) {
        std::cerr << "PostgreSQL error: " << e.what() << std::endl;
        return 1;
    } catch (const sw::redis::Error& e) {
        std::cerr << "Redis error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}