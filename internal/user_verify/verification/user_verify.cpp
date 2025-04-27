#include "user_verify.h"
#include <iostream>

UserVerifier::UserVerifier(pqxx::connection& conn)
    : user_storage_(conn) {}

std::string UserVerifier::GenerateToken(const std::string& email, const std::string& password_hash) {
    User user = user_storage_.GetUserByEmail(email);
    
    if (user.id.empty() || !user_storage_.VerifyPassword(user, password_hash)) {
        std::cout << "Неверный email или пароль при генерации токена." << std::endl;
        return "";
    }

    return token_generator_.GenerateToken(user);
}