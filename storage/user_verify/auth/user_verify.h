#ifndef USER_STORAGE_H
#define USER_STORAGE_H

#include <pqxx/pqxx>
#include "../../auth/user_verify/models/user.h"

/**
 * @brief Класс для взаимодействия с хранилищем пользователей в базе данных.
 *
 * Предоставляет методы для получения информации о пользователях и верификации паролей.
 */
class UserStorage {
public:
    /**
     * @brief Конструктор для UserStorage.
     *
     * @param conn Ссылка на объект pqxx::connection, используемый для взаимодействия с базой данных.
     */
    UserStorage(pqxx::connection& conn);
    /**
     * @brief Получает информацию о пользователе по адресу электронной почты.
     *
     * @param email Адрес электронной почты пользователя.
     * @return Объект User, содержащий данные пользователя, или пустой объект User, если пользователь не найден.
     */
    User GetUserByEmail(const std::string& email);
    /**
     * @brief Проверяет совпадение хеша пароля пользователя.
     *
     * @param user Объект User, содержащий данные пользователя.
     * @param password_hash Хеш пароля для проверки.
     * @return true, если хеши совпадают, false в противном случае.
     */
    bool VerifyPassword(const User& user, const std::string& password_hash);

private:
    pqxx::connection& conn_;
};

#endif