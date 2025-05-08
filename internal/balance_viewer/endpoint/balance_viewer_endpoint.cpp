#include "balance_viewer_endpoint.h"
#include <nlohmann/json.hpp>

std::function<crow::response(const crow::request&)> create_balance_viewer_handler(BalanceViewer& handler) {
    return [&handler](const crow::request& req) {
        try {
            nlohmann::json request_body;
            try {
                request_body = nlohmann::json::parse(req.body);
            } catch (const nlohmann::json::parse_error& e) {
                nlohmann::json error_response = {
                    {"error", "Invalid JSON format"},
                    {"details", e.what()}
                };
                return crow::response(400, error_response.dump());
            }

            nlohmann::json response = handler.HandleRequest(request_body);
            
            if (response.contains("error")) {
                int status_code = 500;
                std::string error_msg = response["error"].get<std::string>();
                
                if (error_msg == "Invalid JSON format") {
                    status_code = 400;
                } else if (error_msg == "Invalid token" || error_msg == "Token not found") {
                    status_code = 401;
                }
                
                return crow::response(status_code, response.dump());
            }
            
            return crow::response(200, response.dump());
            
        } catch (const std::exception& e) {
            nlohmann::json error_response = {
                {"error", "Internal server error"},
                {"details", e.what()}
            };
            return crow::response(500, error_response.dump());
        }
    };
}