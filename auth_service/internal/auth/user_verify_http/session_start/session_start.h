#pragma once

#include <nlohmann/json.hpp>
#include <string>

#include "../../user_verify/verification/user_verify.h"

using json = nlohmann::json;

/**
 * @brief Класс для обработки запросов на начало сессии (аутентификацию).
 *
 * Инкапсулирует логику аутентификации пользователя и генерации токена сессии.
 */
class SessionStart {
 public:
  /**
   * @brief Конструктор класса SessionStart.
   *
   * @param user_verifier Ссылка на объект UserVerifier для проверки учетных
   * данных пользователей.
   */
  explicit SessionStart(UserVerifier& user_verifier);

  /**
   * @brief Обрабатывает запрос на начало сессии (аутентификацию).
   *
   * @param request_data Входящие данные запроса в формате JSON, содержащие
   * "email" и "password_hash".
   * @return JSON-объект с токеном или сообщением об ошибке.
   */
  json HandleRequest(const json& request_data);

 private:
  UserVerifier& user_verifier_;
};