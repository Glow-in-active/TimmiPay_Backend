#ifndef USER_VERIFY_H
#define USER_VERIFY_H

#include <string>
#include "../token_generator/token_generator.h"
#include "../../storage/user_verify/auth/user_verify.h"

class UserVerifier {
    public:
        UserVerifier(pqxx::connection& pg_conn, sw::redis::Redis& redis);
        
        std::string GenerateToken(const std::string& email, 
                                const std::string& password_hash);
    private:
        UserStorage user_storage_;
        UUIDGenerator uuid_generator_;
        sw::redis::Redis& redis_;
        TokenGenerator token_gen_;
    };

#endif