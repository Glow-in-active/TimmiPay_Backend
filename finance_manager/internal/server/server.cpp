#include "server.h"

#include <crow.h>

#include <algorithm>
#include <memory>
#include <nlohmann/json.hpp>
#include <pqxx/pqxx>
#include <string>
#include <vector>

#include "../../../storage/config/config.h"
#include "../../../storage/postgres_connect/connect.h"
#include "../../../storage/session_verify/session_verify.h"
#include "../finance/finance_service.h"

/**
 * @brief Проверяет валидность токена сессии.
 *
 * Использует SessionVerifier для проверки токена сессии и извлечения ID
 * пользователя.
 *
 * @param session_token Токен сессии для проверки.
 * @param user_id Ссылка на строку, в которую будет записан ID пользователя,
 * если сессия действительна.
 * @return true, если сессия действительна, false в противном случае.
 */
bool FinanceServer::verify_session(const std::string& session_token,
                                   std::string& user_id) {
  return session_verifier->verify_session(session_token, user_id);
}

/**
 * @brief Конструктор для FinanceServer.
 *
 * Инициализирует FinanceServer с предоставленными соединениями с PostgreSQL и
 * Redis, а также настраивает маршруты API для получения баланса, перевода денег
 * и получения истории транзакций.
 *
 * @param postgres Ссылка на активное соединение с базой данных PostgreSQL.
 * @param redis Ссылка на активное соединение с Redis.
 *
 * @section balance_endpoint Баланс пользователя (/api/v1/balance)
 * Обрабатывает POST-запросы для получения баланса пользователя. Требует
 * `session_token` в теле запроса. Возвращает массив объектов, каждый из которых
 * содержит `currency` и `balance`. Возвращает 401, если токен сессии
 * недействителен, или 500 в случае внутренней ошибки сервера.
 *
 * @section transfer_endpoint Перевод денег (/api/v1/transfer)
 * Обрабатывает POST-запросы для перевода денег между пользователями. Требует
 * `session_token`, `to_username`, `amount` и `currency` в теле запроса.
 * Возвращает `transfer_id` при успешном выполнении. Возвращает 401, если токен
 * сессии недействителен, 400 в случае ошибки бизнес-логики (например,
 * недостаток средств), или 500 в случае внутренней ошибки сервера.
 *
 * @section history_endpoint История транзакций (/api/v1/history)
 * Обрабатывает POST-запросы для получения истории транзакций пользователя.
 * Требует `session_token` в теле запроса. Поддерживает необязательные параметры
 * `page` и `limit` для пагинации. Возвращает массив объектов, каждый из которых
 * содержит `transfer_id`, `amount`, `status` и `created_at`. Возвращает 401,
 * если токен сессии недействителен, или 500 в случае внутренней ошибки сервера.
 */
FinanceServer::FinanceServer(pqxx::connection& postgres,
                             sw::redis::Redis& redis)
    : db_conn(postgres) {
  try {
    session_verifier = std::make_shared<SessionVerifier>(redis);
    finance_service = std::make_shared<FinanceService>(db_conn);
  } catch (const std::exception& e) {
    throw std::runtime_error("Failed to initialize: " + std::string(e.what()));
  }

  CROW_ROUTE(app, "/api/v1/balance")
      .methods("POST"_method)([this](const crow::request& req) {
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
            response.push_back({{"currency", currency}, {"balance", balance}});
          }

          return crow::response(200, response.dump());
        } catch (const std::exception& e) {
          return crow::response(500, "Internal server error");
        }
      });

  CROW_ROUTE(app, "/api/v1/transfer")
      .methods("POST"_method)([this](const crow::request& req) {
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
                from_user_id, to_username, amount, currency);

            return crow::response(
                200, nlohmann::json{{"transfer_id", transfer_id}}.dump());
          } catch (const std::runtime_error& e) {
            return crow::response(400, e.what());
          }
        } catch (const std::exception& e) {
          return crow::response(500,
                                nlohmann::json{{"error", e.what()}}.dump());
        }
      });

  CROW_ROUTE(app, "/api/v1/history")
      .methods("POST"_method)([this](const crow::request& req) {
        try {
          std::string cleaned_body = req.body;
          cleaned_body.erase(
              std::remove_if(
                  cleaned_body.begin(), cleaned_body.end(),
                  [](unsigned char c) { return c < 0x20 || c == 0x7F; }),
              cleaned_body.end());

          auto body = nlohmann::json::parse(cleaned_body);
          std::string session_token = body["session_token"];
          int page = body.count("page") ? body["page"].get<int>() : 1;
          int limit = body.count("limit") ? body["limit"].get<int>() : 10;

          std::string user_id;
          if (!verify_session(session_token, user_id)) {
            return crow::response(401, "Invalid session token");
          }

          auto transfers =
              finance_service->get_transaction_history(user_id, page, limit);
          nlohmann::json response = nlohmann::json::array();

          for (const auto& transfer : transfers) {
            response.push_back({{"transfer_id", transfer.id},
                                {"amount", transfer.amount},
                                {"status", transfer.status},
                                {"created_at", transfer.created_at}});
          }

          return crow::response(200, response.dump());
        } catch (const std::exception& e) {
          return crow::response(500,
                                nlohmann::json{{"error", e.what()}}.dump());
        }
      });
}

/**
 * @brief Запускает сервер Crow на указанном порту.
 *
 * Сервер будет работать в многопоточном режиме.
 *
 * @param port Номер порта, на котором будет запущен сервер.
 */
void FinanceServer::run(int port) { app.port(port).multithreaded().run(); }

/**
 * @brief Останавливает сервер Crow.
 *
 * Завершает работу приложения Crow.
 */
void FinanceServer::stop_server() { app.stop(); }