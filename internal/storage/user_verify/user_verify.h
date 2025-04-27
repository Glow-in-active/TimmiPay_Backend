#ifndef USER_STORAGE_H
#define USER_STORAGE_H

#include <string>
#include <pqxx/pqxx>
#include "../config/config.h"

struct User {
    std::string id;
    std::string email;
    std::string password_hash;

    User() : id("") {}
};

class UserStorage {
public:
    UserStorage(const Config& config);

    User GetUserByEmail(const std::string& email);
    bool VerifyPassword(const User& user, const std::string& password_hash);

private:
    Config config_;
    pqxx::connection conn_;
};

#endif
