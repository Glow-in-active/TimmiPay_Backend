#include "registration_endpoint.h"

#include <nlohmann/json.hpp>

#include "../../../../auth_service/internal/models/user.h"

/**
 * @brief Создает обработчик HTTP-запросов для регистрации нового пользователя.
 *
 * Этот обработчик принимает запрос Crow, извлекает тело запроса в формате JSON,
 * проверяет наличие имени пользователя, электронной почты и хеша пароля,
 * проверяет, существует ли уже пользователь с такими данными, и, если нет,
 * создает нового пользователя в базе данных.
 *
 * @param user_storage Объект UserStorage, который обрабатывает логику
 * взаимодействия с хранилищем пользователей.
 * @return Функция, которая принимает `crow::request` и возвращает
 * `crow::response`.
 */
std::function<crow::response(const crow::request&)> create_registration_handler(
    UserStorage& user_storage) {
  return [&user_storage](const crow::request& req) {
    try {
      nlohmann::json request_body = nlohmann::json::parse(req.body);

      if (!request_body.contains("username") ||
          !request_body.contains("email") ||
          !request_body.contains("password_hash")) {
        return crow::response(400, nlohmann::json{{"error", "Missing required fields"}}.dump());
      }

      std::string username = request_body["username"].get<std::string>();
      std::string email = request_body["email"].get<std::string>();
      std::string password_hash = request_body["password_hash"].get<std::string>();

      User existing_user_by_email = user_storage.GetUserByEmail(email);
      User existing_user_by_username = user_storage.GetUserByUsername(username);

      if (!existing_user_by_email.email.empty() || !existing_user_by_username.username.empty()) {
        return crow::response(409, nlohmann::json{{"error", "Пользователь с такими данными уже существует"}}.dump());
      }

      if (user_storage.CreateUser(username, email, password_hash)) {
        return crow::response(200, nlohmann::json{{"message", "User registered successfully"}}.dump());
      } else {
        return crow::response(500, nlohmann::json{{"error", "Failed to register user"}}.dump());
      }

    } catch (const nlohmann::json::parse_error& e) {
      return crow::response(400, nlohmann::json{{"error", "Invalid JSON format"}}.dump());
    } catch (const std::exception& e) {
      return crow::response(500, nlohmann::json{{"error", "Internal server error"}}.dump());
    }
  };
} 