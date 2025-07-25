#include "session_refresh_endpoint.h"

#include <nlohmann/json.hpp>

/**
 * @brief Создает обработчик HTTP-запросов для обновления сессии.
 *
 * Этот обработчик принимает запрос Crow, извлекает тело запроса в формате JSON,
 * передает его обработчику `SessionHold` и формирует HTTP-ответ в зависимости
 * от результата. Обрабатывает различные ошибки, такие как неверный формат JSON,
 * ненайденный или истекший токен и внутренние ошибки сервера.
 *
 * @param handler Объект SessionHold, который обрабатывает логику
 * удержания/обновления сессии.
 * @return Функция, которая принимает `crow::request` и возвращает
 * `crow::response`.
 */
std::function<crow::response(const crow::request&)>
create_session_refresh_handler(SessionHold& handler) {
  return [&handler](const crow::request& req) {
    try {
      nlohmann::json request_body = nlohmann::json::parse(req.body);

      nlohmann::json response = handler.HandleRequest(request_body);

      if (response.contains("error")) {
        int status_code = 500;

        if (response["error"] == "Invalid JSON format") {
          status_code = 400;
        } else if (response["error"] == "Token not found or expired") {
          status_code = 404;
        }

        return crow::response(status_code, response.dump());
      }

      return crow::response(200, response.dump());

    } catch (const nlohmann::json::exception& e) {
      return crow::response(
          400, nlohmann::json{{"error", "Invalid JSON format"}}.dump());
    } catch (const std::exception& e) {
      return crow::response(
          500, nlohmann::json{{"error", "Internal server error"}}.dump());
    }
  };
}
