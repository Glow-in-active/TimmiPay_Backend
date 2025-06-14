#include "session_auth_endpoint.h"

#include <nlohmann/json.hpp>

/**
 * @brief Создает обработчик HTTP-запросов для аутентификации сессии.
 *
 * Этот обработчик принимает запрос Crow, извлекает тело запроса в формате JSON,
 * передает его обработчику `SessionStart` и формирует HTTP-ответ в зависимости
 * от результата. Обрабатывает различные ошибки, такие как неверный формат JSON,
 * неудачная верификация и внутренние ошибки сервера.
 *
 * @param handler Объект SessionStart, который обрабатывает логику начала
 * сессии.
 * @return Функция, которая принимает `crow::request` и возвращает
 * `crow::response`.
 */
std::function<crow::response(const crow::request&)> create_session_auth_handler(
    SessionStart& handler) {
  return [&handler](const crow::request& req) {
    try {
      nlohmann::json request_body = nlohmann::json::parse(req.body);
      nlohmann::json response = handler.HandleRequest(request_body);

      if (response.contains("error")) {
        int status_code = 500;
        if (response["error"] == "Invalid JSON format")
          status_code = 400;
        else if (response["error"] == "Verification failed")
          status_code = 401;

        return crow::response(status_code, response.dump());
      }
      return crow::response(200, response.dump());

    } catch (const std::exception& e) {
      return crow::response(
          500, nlohmann::json{{"error", "Internal server error"}}.dump());
    }
  };
}