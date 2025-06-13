#include "session_start.h"
#include <stdexcept>

/**
 * @brief Конструктор класса SessionStart.
 *
 * Инициализирует SessionStart с необходимым верификатором пользователей.
 *
 * @param user_verifier Ссылка на объект UserVerifier для проверки учетных данных пользователей.
 */
SessionStart::SessionStart(UserVerifier& user_verifier)
    : user_verifier_(user_verifier) {}

/**
 * @brief Обрабатывает запрос на начало сессии (аутентификацию).
 *
 * Извлекает email и хеш пароля из данных запроса, использует `UserVerifier` для генерации токена.
 * Возвращает токен в случае успеха или ошибку при неверном формате JSON или неудачной верификации.
 *
 * @param request_data Входящие данные запроса в формате JSON, содержащие "email" и "password_hash".
 * @return JSON-объект с токеном или сообщением об ошибке.
 */
json SessionStart::HandleRequest(const json& request_data) {
    try {
        const std::string email = request_data.at("email").get<std::string>();
        const std::string password_hash = request_data.at("password_hash").get<std::string>();

        const std::string token = user_verifier_.GenerateToken(email, password_hash);
        
        return json{{"token", token}};
        
    } catch (const json::exception& e) {
        return json{
            {"error", "Invalid JSON format"},
            {"details", e.what()}
        };
    } catch (const std::exception& e) {
        return json{
            {"error", "Verification failed"},
            {"details", e.what()}
        };
    }
}