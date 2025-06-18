#pragma once

#include <crow.h>

#include "../../../../storage/user_verify/auth/user_verify.h"

/**
 * @brief Создает обработчик HTTP-запросов для регистрации нового пользователя.
 *
 * @param user_storage Объект UserStorage, который обрабатывает логику
 * взаимодействия с хранилищем пользователей.
 * @return Функция, которая принимает `crow::request` и возвращает
 * `crow::response`.
 */
std::function<crow::response(const crow::request&)> create_registration_handler(
    UserStorage& user_storage); 