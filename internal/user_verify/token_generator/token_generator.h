#ifndef TOKEN_GENERATOR_H
#define TOKEN_GENERATOR_H

#include "../../models/user.h"

class TokenGenerator {
public:
    std::string GenerateToken(const User& user);
};

#endif