#include "session_hold.h"
#include "../../../../storage/user_verify/redis_set/redis_set_token.h"
#include <stdexcept>
#include <nlohmann/json.hpp>

SessionHold::SessionHold(sw::redis::Redis& redis) : redis_(redis) {}

nlohmann::json SessionHold::HandleRequest(const nlohmann::json& request_data) {
    try {
        const std::string token = request_data.at("token").get<std::string>();
        
        hold_token(redis_, token);
        
        if (!redis_.exists(token)) {
            return nlohmann::json{{"error", "Token not found or expired"}};
        }
        
        return nlohmann::json{{"status", "success"}};
        
    } catch (const nlohmann::json::exception& e) {
        return nlohmann::json{
            {"error", "Invalid JSON format"},
            {"details", e.what()}
        };
    } catch (const std::exception& e) {
        return nlohmann::json{
            {"error", "Failed to hold token"},
            {"details", e.what()}
        };
    }
}