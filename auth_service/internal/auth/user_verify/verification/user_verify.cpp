#include "user_verify.h"

#include <iostream>

/**
 * @brief Конструктор класса UserVerifier.
 *
 * Инициализирует UserVerifier с необходимыми зависимостями для работы с
 * PostgreSQL и Redis, а также для генерации токенов.
 *
 * @param pg_conn Ссылка на объект pqxx::connection для взаимодействия с
 * PostgreSQL.
 * @param redis Ссылка на объект sw::redis::Redis для взаимодействия с Redis.
 */
UserVerifier::UserVerifier(pqxx::connection& pg_conn, sw::redis::Redis& redis)
    : user_storage_(pg_conn),
      uuid_generator_(),
      redis_(redis),
      token_gen_(uuid_generator_, redis) {}

/**
 * @brief Генерирует токен аутентификации для пользователя.
 *
 * Проверяет учетные данные пользователя (email и хеш пароля) и, в случае
 * успеха, генерирует новый токен.
 *
 * @param email Электронная почта пользователя.
 * @param password_hash Хеш пароля пользователя.
 * @return Сгенерированный токен аутентификации.
 * @throws std::runtime_error Если логин или пароль неверны.
 */
std::string UserVerifier::GenerateToken(const std::string& email,
                                        const std::string& password_hash) {
  User user = user_storage_.GetUserByEmail(email);

  if (user.email.empty() ||
      !user_storage_.VerifyPassword(user, password_hash)) {
    return "";  // Верификация не удалась
  }

  return token_gen_.GenerateToken(user);
}

UserStorage& UserVerifier::GetUserStorage() {
  return user_storage_;
}
