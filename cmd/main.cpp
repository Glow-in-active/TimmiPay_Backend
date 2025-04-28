#include "../internal/storage/config/config.h"
#include "../internal/storage/postgres_connect/connect.h"
#include "../internal/storage/user_verify/auth/user_verify.h"
#include "../internal/user_verify/verification/user_verify.h"
#include "../internal/storage/redis_config/config_redis.h"
#include "../internal/storage/redis_connect/connect_redis.h"
#include <pqxx/pqxx>
#include <iostream>
#include <string>

int main() {
    try {
        // Загружаем оба конфига
        Config postgres_config = load_config("postgres-config.json");
        ConfigRedis redis_config = load_redis_config("redis-config.json"); // Новый файл конфига
        
        // Устанавливаем оба соединения
        pqxx::connection conn = connect_to_database(postgres_config);
        sw::redis::Redis redis_conn = connect_to_redis(redis_config);  // Подключаемся к Redis
        
        // Передаем оба соединения в верификатор
        UserVerifier user_verifier(conn, redis_conn);  // Нужно обновить конструктор

        std::string email;
        std::string password_hash;

        std::cout << "Введите email: ";
        std::getline(std::cin, email);

        std::cout << "Введите hash пароля: ";
        std::getline(std::cin, password_hash);

        std::string token = user_verifier.GenerateToken(email, password_hash);

        if (!token.empty()) {
            std::cout << "Пользователь успешно верифицирован. Токен: " << token << std::endl;
            // Токен уже сохранен в Redis через set_token()
        } else {
            std::cout << "Неверный email или пароль." << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}