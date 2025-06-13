#include "session_verify.h"
#include <stdexcept>

SessionVerifier::SessionVerifier(sw::redis::Redis& redis) : redis_client(redis) {}

bool SessionVerifier::verify_session(const std::string& session_token, std::string& user_id) {
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

bool SessionVerifier::set_session(const std::string& user_id, const std::string& session_token) {
    try {
        redis_client.hset(session_token, "id", user_id);
        // Устанавливаем время жизни сессии (например, 24 часа)
        redis_client.expire(session_token, 24 * 60 * 60);
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool SessionVerifier::remove_session(const std::string& session_token) {
    try {
        redis_client.hdel(session_token, "id");
        return true;
    } catch (const std::exception& e) {
        return false;
    }
} 