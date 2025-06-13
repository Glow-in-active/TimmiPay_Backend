#include <crow.h>
#include <pqxx/pqxx>
#include <memory>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <algorithm>
#include "../../../storage/session_verify/session_verify.h"
#include "../../../storage/config/config.h"
#include "../../../storage/postgres_connect/connect.h"
#include "../finance/finance_service.h"

class FinanceServer {
private:
    crow::SimpleApp app;
    pqxx::connection& db_conn;
    std::shared_ptr<SessionVerifier> session_verifier;
    std::shared_ptr<FinanceService> finance_service;

    bool verify_session(const std::string& session_token, std::string& user_id) {
        return session_verifier->verify_session(session_token, user_id);
    }

public:
    FinanceServer(pqxx::connection& postgres, sw::redis::Redis& redis) 
        : db_conn(postgres) {
        try {
            session_verifier = std::make_shared<SessionVerifier>(redis);
            finance_service = std::make_shared<FinanceService>(db_conn);
        } catch (const std::exception& e) {
            throw std::runtime_error("Failed to initialize: " + std::string(e.what()));
        }

        // Get balance endpoint
        CROW_ROUTE(app, "/api/v1/balance")
        .methods("POST"_method)
        ([this](const crow::request& req) {
            try {
                auto body = nlohmann::json::parse(req.body);
                std::string session_token = body["session_token"];

                std::string user_id;
                if (!verify_session(session_token, user_id)) {
                    return crow::response(401, "Invalid session token");
                }

                auto balances = finance_service->get_user_balance(user_id);
                nlohmann::json response = nlohmann::json::array();
                
                for (const auto& [currency, balance] : balances) {
                    response.push_back({
                        {"currency", currency},
                        {"balance", balance}
                    });
                }

                return crow::response(200, response.dump());
            } catch (const std::exception& e) {
                return crow::response(500, "Internal server error");
            }
        });

        // Transfer money endpoint
        CROW_ROUTE(app, "/api/v1/transfer")
        .methods("POST"_method)
        ([this](const crow::request& req) {
            try {
                auto body = nlohmann::json::parse(req.body);
                std::string session_token = body["session_token"];
                std::string to_username = body["to_username"];
                double amount = body["amount"];
                std::string currency = body["currency"];

                std::string from_user_id;
                if (!verify_session(session_token, from_user_id)) {
                    return crow::response(401, "Invalid session token");
                }

                try {
                    std::string transfer_id = finance_service->transfer_money(
                        from_user_id,
                        to_username,
                        amount,
                        currency
                    );

                    return crow::response(200, nlohmann::json{{"transfer_id", transfer_id}}.dump());
                } catch (const std::runtime_error& e) {
                    return crow::response(400, e.what());
                }
            } catch (const std::exception& e) {
                return crow::response(500, nlohmann::json{{"error", e.what()}}.dump());
            }
        });

        // Get transaction history endpoint
        CROW_ROUTE(app, "/api/v1/history")
        .methods("POST"_method)
        ([this](const crow::request& req) {
            try {
                // Clean the request body by removing non-printable ASCII characters and null characters
                std::string cleaned_body = req.body;
                cleaned_body.erase(std::remove_if(cleaned_body.begin(), cleaned_body.end(),
                                                [](unsigned char c) { return c < 0x20 || c == 0x7F; }),
                                cleaned_body.end());
                
                auto body = nlohmann::json::parse(cleaned_body);
                std::string session_token = body["session_token"];

                std::string user_id;
                if (!verify_session(session_token, user_id)) {
                    return crow::response(401, "Invalid session token");
                }

                auto transfers = finance_service->get_transaction_history(user_id);
                nlohmann::json response = nlohmann::json::array();
                
                for (const auto& transfer : transfers) {
                    response.push_back({
                        {"transfer_id", transfer.id},
                        {"amount", transfer.amount},
                        {"status", transfer.status},
                        {"created_at", transfer.created_at}
                    });
                }

                return crow::response(200, response.dump());
            } catch (const std::exception& e) {
                return crow::response(500, nlohmann::json{{"error", e.what()}}.dump());
            }
        });
    }

    void run(int port) {
        app.port(port).multithreaded().run();
    }
}; 