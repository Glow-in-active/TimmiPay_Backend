#ifndef USER_STORAGE_H
#define USER_STORAGE_H

#include <pqxx/pqxx>

#include "../../../auth_service/internal/models/user.h"

/**
 * @brief Класс для взаимодействия с хранилищем пользователей в базе данных.
 *
 * Предоставляет методы для получения информации о пользователях и верификации
 * паролей.
 */
class UserStorage {
 public:
  /**
   * @brief Конструктор для UserStorage.
   *
   * @param conn Ссылка на объект pqxx::connection, используемый для
   * взаимодействия с базой данных.
   */
  UserStorage(pqxx::connection& conn);
  /**
   * @brief Получает информацию о пользователе по адресу электронной почты.
   *
   * @param email Адрес электронной почты пользователя.
   * @return Объект User, содержащий данные пользователя, или пустой объект
   * User, если пользователь не найден.
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

  /**
   * @brief Получает информацию о пользователе по имени пользователя.
   *
   * @param username Имя пользователя.
   * @return Объект User, содержащий данные пользователя, или пустой объект
   * User, если пользователь не найден.
   */
  User GetUserByUsername(const std::string& username);

  /**
   * @brief Создает нового пользователя в базе данных.
   *
   * @param username Имя пользователя.
   * @param email Адрес электронной почты пользователя.
   * @param password_hash Хеш пароля пользователя.
   * @return true, если пользователь успешно создан, false в противном случае.
   */
  bool CreateUser(const std::string& username, const std::string& email,
                  const std::string& password_hash);

 private:
  pqxx::connection& conn_;
};

#endif