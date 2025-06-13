#pragma once

#include "../dependencies/dependencies.h"
#include <crow.h>

/**
 * @brief Запускает HTTP-сервер Crow для сервиса аутентификации.
 *
 * @param deps Структура Dependencies, содержащая обработчики для начала и удержания сессий.
 */
void start_server(Dependencies& deps);
