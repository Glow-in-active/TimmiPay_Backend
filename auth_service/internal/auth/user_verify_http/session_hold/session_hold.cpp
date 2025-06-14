#include "session_hold.h"

#include <nlohmann/json.hpp>
#include <stdexcept>

#include "../../../../storage/user_verify/redis_set/redis_set_token.h"

/**
 * @brief Конструктор класса SessionHold.
 *
 * Инициализирует SessionHold с необходимым соединением Redis.
 *
 * @param redis Ссылка на объект sw::redis::Redis для взаимодействия с Redis.
 */
SessionHold::SessionHold(sw::redis::Redis& redis) : redis_(redis) {}

/**
 * @brief Обрабатывает запрос на удержание (обновление) токена сессии.
 *
 * Извлекает токен из данных запроса, вызывает `hold_token` для обновления
 * времени жизни токена в Redis. Возвращает успешный ответ, если токен
 * существует, или ошибку, если токен не найден, истек или формат JSON неверен.
 *
 * @param request_data Входящие данные запроса в формате JSON, содержащие поле
 * "token".
 * @return JSON-объект с результатом операции (status: "success" или error:
 * "...").
 */
nlohmann::json SessionHold::HandleRequest(const nlohmann::json& request_data) {
  try {
    const std::string token = request_data.at("token").get<std::string>();

    hold_token(redis_, token);

    if (!redis_.exists(token)) {
      return nlohmann::json{{"error", "Token not found or expired"}};
    }

    return nlohmann::json{{"status", "success"}};

  } catch (const nlohmann::json::exception& e) {
    return nlohmann::json{{"error", "Invalid JSON format"},
                          {"details", e.what()}};
  } catch (const std::exception& e) {
    return nlohmann::json{{"error", "Failed to hold token"},
                          {"details", e.what()}};
  }
}