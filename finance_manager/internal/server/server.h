#ifndef FINANCE_SERVER_H
#define FINANCE_SERVER_H

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

namespace sw { namespace redis { class Redis; } }

/**
 * @brief Класс, представляющий финансовый сервер на базе Crow.
 *
 * Обрабатывает HTTP-запросы, связанные с финансовыми операциями, такими как получение баланса,
 * перевод денег и получение истории транзакций.
 */
class FinanceServer {
private:
    crow::SimpleApp app;
    pqxx::connection& db_conn;
    std::shared_ptr<SessionVerifier> session_verifier;
    std::shared_ptr<FinanceService> finance_service;

    /**
     * @brief Проверяет валидность токена сессии.
     *
     * @param session_token Токен сессии для проверки.
     * @param user_id Ссылка на строку, в которую будет записан ID пользователя, если сессия действительна.
     * @return true, если сессия действительна, false в противном случае.
     */
    bool verify_session(const std::string& session_token, std::string& user_id);

public:
    /**
     * @brief Конструктор для FinanceServer.
     *
     * Инициализирует FinanceServer с предоставленными соединениями с PostgreSQL и Redis,
     * а также настраивает маршруты API.
     *
     * @param postgres Ссылка на активное соединение с базой данных PostgreSQL.
     * @param redis Ссылка на активное соединение с Redis.
     */
    FinanceServer(pqxx::connection& postgres, sw::redis::Redis& redis);

    /**
     * @brief Запускает сервер Crow на указанном порту.
     *
     * @param port Номер порта, на котором будет запущен сервер.
     */
    void run(int port);

    /**
     * @brief Останавливает сервер Crow.
     */
    void stop_server();
};

#endif // FINANCE_SERVER_H 