#include "session_start.h"
#include <stdexcept>

SessionStart::SessionStart(UserVerifier& user_verifier)
    : user_verifier_(user_verifier) {}

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