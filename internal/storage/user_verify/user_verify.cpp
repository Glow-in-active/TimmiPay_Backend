#include "user_verify.h"
#include <iostream>
#include "../postgres_connect/connect.h"
#include <string>

UserStorage::UserStorage(const Config& config) : config_(config), conn_(connect_to_database(config)) {}

User UserStorage::GetUserByEmail(const std::string& email) {
    User user;
    try {
        pqxx::nontransaction transaction(conn_);
        pqxx::result result = transaction.exec_params(
            "SELECT id, email, password_hash FROM users WHERE email = $1",
            email
        );

        if (!result.empty()) {
            user.id = result[0]["id"].as<std::string>();
            user.email = result[0]["email"].as<std::string>();
            user.password_hash = result[0]["password_hash"].as<std::string>();
        }
    } catch (const std::exception& e) {
        std::cerr << "Ошибка при получении пользователя из базы данных: " << e.what() << std::endl;
    }
    return user;
}

bool UserStorage::VerifyPassword(const User& user, const std::string& password_hash) {
    return user.password_hash == password_hash;
}
