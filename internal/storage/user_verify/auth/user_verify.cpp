#include "user_verify.h"
#include <pqxx/pqxx>
#include <iostream>

UserStorage::UserStorage(pqxx::connection& conn) : conn_(conn) {}

User UserStorage::GetUserByEmail(const std::string& email) {
    try {
        pqxx::work transaction(conn_);
        pqxx::result result = transaction.exec_params(
            "SELECT id, email, password_hash FROM users WHERE email = $1",
            email
        );

        if (result.empty()) return User{};

        return User{
            result[0][0].as<std::string>(),
            result[0][1].as<std::string>(),
            result[0][2].as<std::string>()
        };
    }
    catch (const std::exception& e) {
        std::cout << "Database error: " << e.what() << std::endl;
        return User{};
    }
}

bool UserStorage::VerifyPassword(const User& user, const std::string& password_hash) {
    return user.password_hash == password_hash;
}