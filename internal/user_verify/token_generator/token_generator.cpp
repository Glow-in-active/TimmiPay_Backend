#include "token_generator.h"
#include <iostream>

std::string TokenGenerator::GenerateToken(const User& user) {
    //логика генерации токена (например, JWT)
    std::cout << "Generating token for user: " << user.email << std::endl;
    return "dummy_token_for_user_" + user.id;
}
