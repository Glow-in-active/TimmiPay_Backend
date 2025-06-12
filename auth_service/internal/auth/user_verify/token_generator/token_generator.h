#ifndef TOKEN_GENERATOR_H
#define TOKEN_GENERATOR_H

#include "../../models/user.h"
#include "../../../../uuid_generator/uuid_generator.h"
#include <sw/redis++/redis++.h>

class TokenGenerator {
public:
    TokenGenerator(UUIDGenerator& uuid_gen, sw::redis::Redis& redis);
    
    std::string GenerateToken(const User& user);

private:
    UUIDGenerator& uuid_gen_;
    sw::redis::Redis& redis_;
};

#endif