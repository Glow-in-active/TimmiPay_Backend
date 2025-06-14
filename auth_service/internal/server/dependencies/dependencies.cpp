#include "dependencies.h"

/**
 * @brief Инициализирует и возвращает структуру зависимостей приложения.
 *
 * Создает экземпляры UserVerifier, SessionStart и SessionHold,
 * используя предоставленные соединения с базами данных.
 *
 * @param db Ссылка на структуру DBConnections, содержащую соединения с
 * PostgreSQL и Redis.
 * @return Структура Dependencies, содержащая инициализированные обработчики.
 */
Dependencies initialize_dependencies(DBConnections& db) {
  UserVerifier user_verifier(db.postgres, db.redis);
  SessionStart session_start_handler(user_verifier);
  SessionHold session_hold_handler(db.redis);

  return {user_verifier, session_start_handler, session_hold_handler};
}
