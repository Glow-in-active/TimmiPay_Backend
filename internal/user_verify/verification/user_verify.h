#ifndef USER_VERIFY_H
#define USER_VERIFY_H

#include <string>
#include "../token_generator/token_generator.h"
#include "../../storage/user_verify/auth/user_verify.h"

class UserVerifier {
public:
    UserVerifier(pqxx::connection& conn);
    std::string GenerateToken(const std::string& email, const std::string& password_hash);

private:
    UserStorage user_storage_;
    TokenGenerator token_generator_;
};

#endif