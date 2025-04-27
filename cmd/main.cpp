#include "../internal/storage/config/config.h"
#include "../internal/storage/postgres_connect/connect.h"
#include "../internal/storage/user_verify/user_verify.h"
#include "../internal/user_verify/verification/user_verify.h"
#include <pqxx/pqxx>
#include <iostream>
#include <string>

int main() {
    try {
        Config config = load_config("postgres-config.json");
        pqxx::connection conn = connect_to_database(config);
        
        UserVerifier user_verifier(conn); 

        std::string email;
        std::string password_hash;

        std::cout << "Введите email: ";
        std::getline(std::cin, email);

        std::cout << "Введите hash пароля: ";
        std::getline(std::cin, password_hash);

        std::string token = user_verifier.GenerateToken(email, password_hash);

        if (!token.empty()) {
            std::cout << "Пользователь успешно верифицирован. Токен: " << token << std::endl;
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