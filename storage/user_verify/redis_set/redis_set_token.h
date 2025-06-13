#pragma once
#include <sw/redis++/redis++.h>
#include <string>

/**
 * @brief Устанавливает токен сессии в Redis.
 *
 * Сохраняет токен сессии и связанный с ним ID пользователя в Redis, устанавливая срок действия.
 *
 * @param redis Ссылка на объект sw::redis::Redis для взаимодействия с Redis.
 * @param token Строка, представляющая токен сессии.
 * @param id Строка, представляющая ID пользователя.
 * @throws std::runtime_error В случае ошибок Redis или системных ошибок.
 */
void set_token(sw::redis::Redis& redis, 
              const std::string& token, 
              const std::string& id);

/**
 * @brief Продлевает срок действия токена сессии в Redis.
 *
 * Если токен сессии существует в Redis, его срок действия обновляется.
 *
 * @param redis Ссылка на объект sw::redis::Redis для взаимодействия с Redis.
 * @param token Строка, представляющая токен сессии.
 * @throws std::runtime_error В случае ошибок Redis или системных ошибок.
 */
void hold_token(sw::redis::Redis& redis, 
               const std::string& token);