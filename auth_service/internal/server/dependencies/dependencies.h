#pragma once

#include "../../auth/user_verify/verification/user_verify.h"
#include "../../auth/user_verify_http/session_hold/session_hold.h"
#include "../../auth/user_verify_http/session_start/session_start.h"
#include "../db_init/db_init.h"

/**
 * @brief Структура, содержащая ключевые зависимости для приложения
 * аутентификации.
 *
 * Включает верификатор пользователей, обработчик начала сессии и обработчик
 * удержания сессии.
 */
struct Dependencies {
  UserVerifier user_verifier;
  SessionStart session_start_handler;
  SessionHold session_hold_handler;
};

/**
 * @brief Инициализирует и возвращает структуру зависимостей приложения.
 *
 * @param db Ссылка на структуру DBConnections, содержащую соединения с
 * PostgreSQL и Redis.
 * @return Структура Dependencies, содержащая инициализированные обработчики.
 */
Dependencies initialize_dependencies(DBConnections& db);
