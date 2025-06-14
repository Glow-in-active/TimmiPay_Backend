#ifndef USER_MODEL_H
#define USER_MODEL_H

#include <string>

/**
 * @brief Структура, представляющая пользователя в системе аутентификации.
 */
struct User {
  std::string id;
  std::string email;
  std::string password_hash;
  std::string username;
};

#endif