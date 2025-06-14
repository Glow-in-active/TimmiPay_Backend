#include "session_verify.h"

#include <stdexcept>

/**
 * @brief Конструктор для SessionVerifier.
 *
 * Инициализирует SessionVerifier с предоставленным клиентом Redis.
 *
 * @param redis Ссылка на объект sw::redis::Redis, используемый для
 * взаимодействия с Redis.
 */
SessionVerifier::SessionVerifier(sw::redis::Redis& redis)
    : redis_client(redis) {}

/**
 * @brief Проверяет сессию по токену.
 *
 * Ищет токен сессии в Redis и извлекает связанный с ним ID пользователя.
 *
 * @param session_token Токен сессии для проверки.
 * @param user_id Ссылка на строку, в которую будет записан ID пользователя,
 * если сессия действительна.
 * @return true, если сессия действительна и ID пользователя получен успешно,
 * false в противном случае.
 */
bool SessionVerifier::verify_session(const std::string& session_token,
                                     std::string& user_id) {
  try {
    auto result = redis_client.hget(session_token, "id");

    if (!result) {
      return false;
    }

    user_id = *result;
    return true;
  } catch (const std::exception& e) {
    return false;
  }
}

/**
 * @brief Устанавливает новую сессию для пользователя.
 *
 * Сохраняет ID пользователя, связанный с токеном сессии, в Redis и
 * устанавливает срок действия для сессии.
 *
 * @param user_id ID пользователя, для которого устанавливается сессия.
 * @param session_token Токен сессии, который будет связан с ID пользователя.
 * @return true, если сессия успешно установлена, false в противном случае.
 */
bool SessionVerifier::set_session(const std::string& user_id,
                                  const std::string& session_token) {
  try {
    redis_client.hset(session_token, "id", user_id);
    redis_client.expire(session_token, 24 * 60 * 60);
    return true;
  } catch (const std::exception& e) {
    return false;
  }
}

/**
 * @brief Удаляет сессию по токену.
 *
 * Удаляет токен сессии и связанные с ним данные из Redis.
 *
 * @param session_token Токен сессии для удаления.
 * @return true, если сессия успешно удалена, false в противном случае.
 */
bool SessionVerifier::remove_session(const std::string& session_token) {
  try {
    redis_client.hdel(session_token, "id");
    return true;
  } catch (const std::exception& e) {
    return false;
  }
}