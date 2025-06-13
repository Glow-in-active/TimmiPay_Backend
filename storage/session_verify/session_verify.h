#pragma once

#include <string>
#include <memory>
#include <sw/redis++/redis++.h>

class SessionVerifier {
public:
    SessionVerifier(sw::redis::Redis& redis);
    
    // Проверяет валидность токена сессии
    bool verify_session(const std::string& session_token, std::string& user_id);
    
    // Устанавливает токен сессии для пользователя
    bool set_session(const std::string& user_id, const std::string& session_token);
    
    // Удаляет токен сессии
    bool remove_session(const std::string& session_token);

private:
    sw::redis::Redis& redis_client;
}; 