#include "redis_set_token.h"
#include <chrono>
#include <stdexcept>
#include <vector>
#include <sw/redis++/utils.h>

void set_token(sw::redis::Redis& redis,
              const std::string& token,
              const std::string& id) {
    try {
        auto expires_at = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count() + 600;

        std::vector<std::pair<sw::redis::StringView, std::string>> fields = {
            {"id", id},
            {"expires_at", std::to_string(expires_at)}
        };
        redis.hset(token, fields.begin(), fields.end());

        redis.expire(token, 600);
        
    } catch (const sw::redis::Error& e) {
        throw std::runtime_error("Redis error: " + std::string(e.what()));
    } catch (const std::exception& e) {
        throw std::runtime_error("System error: " + std::string(e.what()));
    }
}

void hold_token(sw::redis::Redis& redis, const std::string& token) {
    try {
        auto ttl = redis.ttl(token);
        
        if (ttl != -2) {
            redis.expire(token, 600);
        }
    } catch (const sw::redis::Error& e) {
        throw std::runtime_error("Redis error: " + std::string(e.what()));
    } catch (const std::exception& e) {
        throw std::runtime_error("System error: " + std::string(e.what()));
    }
}