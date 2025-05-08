#include "redis_get_id_by_token.h"
#include <stdexcept>

std::string get_id_by_token(sw::redis::Redis& redis,
                           const std::string& token) {
    try {
        auto id_value = redis.hget(token, "id");
        
        if (!id_value) {
            throw std::runtime_error("Token not found or id field missing");
        }
        
        return *id_value;
        
    } catch (const sw::redis::Error& e) {
        throw std::runtime_error("Redis error: " + std::string(e.what()));
    } catch (const std::exception& e) {
        throw std::runtime_error("System error: " + std::string(e.what()));
    }
}