#pragma once

#include <sw/redis++/redis++.h>

#include <memory>
#include <string>

/**
 * @brief Класс для верификации и управления сессиями пользователей в Redis.
 */
class SessionVerifier {
 public:
  /**
   * @brief Конструктор для SessionVerifier.
   *
   * @param redis Ссылка на объект sw::redis::Redis, используемый для
   * взаимодействия с Redis.
   */
  SessionVerifier(sw::redis::Redis& redis);

  /**
   * @brief Проверяет сессию по токену.
   *
   * @param session_token Токен сессии для проверки.
   * @param user_id Ссылка на строку, в которую будет записан ID пользователя,
   * если сессия действительна.
   * @return true, если сессия действительна и ID пользователя получен успешно,
   * false в противном случае.
   */
  bool verify_session(const std::string& session_token, std::string& user_id);

  /**
   * @brief Устанавливает новую сессию для пользователя.
   *
   * @param user_id ID пользователя, для которого устанавливается сессия.
   * @param session_token Токен сессии, который будет связан с ID пользователя.
   * @return true, если сессия успешно установлена, false в противном случае.
   */
  bool set_session(const std::string& user_id,
                   const std::string& session_token);

  /**
   * @brief Удаляет сессию по токену.
   *
   * @param session_token Токен сессии для удаления.
   * @return true, если сессия успешно удалена, false в противном случае.
   */
  bool remove_session(const std::string& session_token);

 private:
  sw::redis::Redis& redis_client;
};