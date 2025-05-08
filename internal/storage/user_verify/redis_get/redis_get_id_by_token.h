#pragma once
#include <sw/redis++/redis++.h>
#include <string>

/**
 * Извлекает идентификатор пользователя по токену из Redis
 * 
 * @param redis Клиент Redis для выполнения команд
 * @param token Токен для поиска
 * @return Строку с идентификатором пользователя
 * @throws std::runtime_error Если токен не найден, отсутствует поле id
 *         или произошла ошибка при обращении к Redis
 */
std::string get_id_by_token(sw::redis::Redis& redis,const std::string& token);