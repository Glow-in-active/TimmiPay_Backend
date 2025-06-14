#ifndef SESSION_HOLD_H
#define SESSION_HOLD_H

#include <sw/redis++/redis++.h>

#include <nlohmann/json.hpp>

/**
 * @brief Класс для обработки запросов на удержание (обновление) сессии.
 *
 * Отвечает за взаимодействие с Redis для обновления времени жизни токенов
 * сессий.
 */
class SessionHold {
 public:
  /**
   * @brief Конструктор класса SessionHold.
   *
   * @param redis Ссылка на объект sw::redis::Redis для взаимодействия с Redis.
   */
  explicit SessionHold(sw::redis::Redis& redis);

  /**
   * @brief Обрабатывает запрос на удержание (обновление) токена сессии.
   *
   * @param request_data Входящие данные запроса в формате JSON, содержащие поле
   * "token".
   * @return JSON-объект с результатом операции.
   */
  nlohmann::json HandleRequest(const nlohmann::json& request_data);
  sw::redis::Redis& redis_;
};

#endif