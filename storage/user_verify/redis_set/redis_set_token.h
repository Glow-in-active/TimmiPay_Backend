#pragma once
#include <sw/redis++/redis++.h>
#include <string>

/**
 * Сохраняет токен и связанные данные в Redis с временем жизни 10 минут
 * 
 * @param redis Клиент Redis для выполнения команд
 * @param token Токен для сохранения (используется как ключ)
 * @param id Идентификатор пользователя
 * @throws std::runtime_error При ошибках записи или установки TTL
 */
void set_token(sw::redis::Redis& redis, 
              const std::string& token, 
              const std::string& id);

/**
 * Продлевает время жизни существующего токена на 10 минут
 * 
 * @param redis Клиент Redis для выполнения команд
 * @param token Токен для продления
 * @throws std::runtime_error При ошибках обновления TTL
 */
void hold_token(sw::redis::Redis& redis, 
               const std::string& token);