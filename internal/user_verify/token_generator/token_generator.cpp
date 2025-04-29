#include "token_generator.h"
#include "../../storage/user_verify/redis_set/redis_set_token.h"
#include "../../storage/user_verify/auth/user_verify.h"

std::string TokenGenerator::GenerateToken(const User& user) {
    const std::string token = uuid_gen_.generateUUID();

    set_token(redis_, token, user.id);
    
    return token;
}

TokenGenerator::TokenGenerator(UUIDGenerator& uuid_gen, sw::redis::Redis& redis)
    : uuid_gen_(uuid_gen), 
      redis_(redis) 
{}