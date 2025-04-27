#include "../internal/storage/config/config.h"
#include "../internal/storage/postgres_connect/connect.h"
#include "../internal/storage/user_verify/user_verify.h"
#include <pqxx/pqxx>
#include <iostream>
#include <string>

int main() {
    try {
        Config config = load_config("postgres-config.json");
        pqxx::connection conn = connect_to_database(config);
        UserStorage user_storage(config);

        std::string email;
        std::string password_hash;

        std::cout << "Введите email: ";
        std::cin >> email;

        std::cout << "Введите hash пароля: ";
        std::cin >> password_hash;

        User user = user_storage.GetUserByEmail(email);

        if (!user.id.empty() && user_storage.VerifyPassword(user, password_hash)) {
            std::cout << "Пользователь успешно верифицирован." << std::endl;
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
