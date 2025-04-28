#ifndef USER_STORAGE_H
#define USER_STORAGE_H

#include <pqxx/pqxx>
#include "../../models/user.h"

class UserStorage {
public:
    UserStorage(pqxx::connection& conn);
    User GetUserByEmail(const std::string& email);
    bool VerifyPassword(const User& user, const std::string& password_hash);

private:
    pqxx::connection& conn_;
};

#endif