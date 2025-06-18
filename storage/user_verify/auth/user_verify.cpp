#include "user_verify.h"

#include <iostream>
#include <pqxx/pqxx>

/**
 * @brief Конструктор для UserStorage.
 *
 * Инициализирует UserStorage с предоставленным соединением с базой данных
 * PostgreSQL.
 *
 * @param conn Ссылка на объект pqxx::connection, используемый для
 * взаимодействия с базой данных.
 */
UserStorage::UserStorage(pqxx::connection& conn) : conn_(conn) {}

/**
 * @brief Получает информацию о пользователе по адресу электронной почты.
 *
 * Выполняет запрос к базе данных для поиска пользователя по его электронной
 * почте.
 *
 * @param email Адрес электронной почты пользователя.
 * @return Объект User, содержащий данные пользователя, или пустой объект User,
 * если пользователь не найден или произошла ошибка.
 */
User UserStorage::GetUserByEmail(const std::string& email) {
  try {
    pqxx::work transaction(conn_);
    pqxx::result result = transaction.exec_params(
        "SELECT id, email, password_hash FROM users WHERE email = $1", email);

    if (result.empty()) return User{};

    return User{result[0][0].as<std::string>(), result[0][1].as<std::string>(),
                result[0][2].as<std::string>()};
  } catch (const std::exception& e) {
    std::cout << "Database error: " << e.what() << std::endl;
    return User{};
  }
}

/**
 * @brief Проверяет совпадение хеша пароля пользователя.
 *
 * Сравнивает предоставленный хеш пароля с хешем пароля, хранящимся в объекте
 * пользователя.
 *
 * @param user Объект User, содержащий данные пользователя.
 * @param password_hash Хеш пароля для проверки.
 * @return true, если хеши совпадают, false в противном случае.
 */
bool UserStorage::VerifyPassword(const User& user,
                                 const std::string& password_hash) {
  return user.password_hash == password_hash;
}

/**
 * @brief Получает информацию о пользователе по имени пользователя.
 *
 * Выполняет запрос к базе данных для поиска пользователя по его имени
 * пользователя.
 *
 * @param username Имя пользователя.
 * @return Объект User, содержащий данные пользователя, или пустой объект User,
 * если пользователь не найден или произошла ошибка.
 */
User UserStorage::GetUserByUsername(const std::string& username) {
  try {
    pqxx::work transaction(conn_);
    pqxx::result result = transaction.exec_params(
        "SELECT id, email, password_hash, username FROM users WHERE username = $1",
        username);

    if (result.empty()) return User{};

    return User{result[0][0].as<std::string>(), result[0][1].as<std::string>(),
                result[0][2].as<std::string>(), result[0][3].as<std::string>()};
  } catch (const std::exception& e) {
    std::cout << "Database error: " << e.what() << std::endl;
    return User{};
  }
}

/**
 * @brief Создает нового пользователя в базе данных.
 *
 * Вставляет нового пользователя с указанным именем пользователя, адресом
 * электронной почты и хешем пароля в таблицу users.
 *
 * @param username Имя пользователя.
 * @param email Адрес электронной почты пользователя.
 * @param password_hash Хеш пароля пользователя.
 * @return true, если пользователь успешно создан, false в противном случае.
 */
bool UserStorage::CreateUser(const std::string& username,
                             const std::string& email,
                             const std::string& password_hash) {
  try {
    pqxx::work transaction(conn_);
    transaction.exec_params(
        "INSERT INTO users (username, email, password_hash) VALUES ($1, $2, "
        "$3)",
        username, email, password_hash);
    transaction.commit();
    return true;
  } catch (const pqxx::unique_violation& e) {
    std::cout << "User with this email or username already exists: " << e.what()
              << std::endl;
    return false;
  } catch (const std::exception& e) {
    std::cout << "Database error: " << e.what() << std::endl;
    return false;
  }
}