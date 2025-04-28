#include "user_verify.h"
#include <iostream>

UserVerifier::UserVerifier(pqxx::connection& pg_conn, sw::redis::Redis& redis) 
    : user_storage_(pg_conn),
      uuid_generator_(),
      redis_(redis),
      token_gen_(uuid_generator_, redis) {}

std::string UserVerifier::GenerateToken(const std::string& email, 
                                      const std::string& password_hash) {
    User user = user_storage_.GetUserByEmail(email);
    if (!user.id.empty() && user_storage_.VerifyPassword(user, password_hash)) {
        return token_gen_.GenerateToken(user);
    }
    return "";
}