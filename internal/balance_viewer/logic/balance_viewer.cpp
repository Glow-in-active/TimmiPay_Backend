#include "balance_viewer.h"
#include "redis_get_id_by_token.h"
#include <stdexcept>

BalanceViewer::BalanceViewer(sw::redis::Redis& redis, BalanceStorage& balance_storage)
    : redis_(redis), balance_storage_(balance_storage) {}

nlohmann::json BalanceViewer::HandleRequest(const nlohmann::json& request_data) {
    try {
        const std::string token = request_data.at("token").get<std::string>();
        
        std::string user_id;
        try {
            user_id = get_id_by_token(redis_, token);
        } catch (const std::runtime_error& e) {
            return nlohmann::json{{"error", e.what()}};
        }
        
        auto balances = balance_storage_.GetUserBalances(user_id);
        
        nlohmann::json result;
        for (const auto& [currency, balance] : balances) {
            result[currency] = balance;
        }
        
        return nlohmann::json{{"balances", result}};
        
    } catch (const nlohmann::json::exception& e) {
        return nlohmann::json{
            {"error", "Invalid JSON format"},
            {"details", e.what()}
        };
    } catch (const std::exception& e) {
        return nlohmann::json{
            {"error", "Internal server error"},
            {"details", e.what()}
        };
    }
}