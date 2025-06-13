#ifndef USER_VERIFY_H
#define USER_VERIFY_H

#include <string>
#include "../token_generator/token_generator.h"
#include "../../../../storage/user_verify/auth/user_verify.h"

/**
 * @brief Класс для верификации пользователей и генерации токенов.
 *
 * Инкапсулирует логику проверки учетных данных пользователя, взаимодействия с базой данных
 * и генерации уникальных токенов сессий.
 */
class UserVerifier {
    public:
        /**
         * @brief Конструктор класса UserVerifier.
         *
         * Инициализирует UserVerifier с необходимыми зависимостями для работы с PostgreSQL и Redis.
         *
         * @param pg_conn Ссылка на объект pqxx::connection для взаимодействия с PostgreSQL.
         * @param redis Ссылка на объект sw::redis::Redis для взаимодействия с Redis.
         */
        UserVerifier(pqxx::connection& pg_conn, sw::redis::Redis& redis);
        
        /**
         * @brief Генерирует токен аутентификации для пользователя.
         *
         * Проверяет учетные данные пользователя и, в случае успеха, генерирует новый токен.
         *
         * @param email Электронная почта пользователя.
         * @param password_hash Хеш пароля пользователя.
         * @return Сгенерированный токен аутентификации.
         */
        std::string GenerateToken(const std::string& email, 
                                const std::string& password_hash);
    private:
        UserStorage user_storage_;
        UUIDGenerator uuid_generator_;
        sw::redis::Redis& redis_;
        TokenGenerator token_gen_;
    };
#endif