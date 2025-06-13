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

// Forward declarations
namespace sw { namespace redis { class Redis; } }

class FinanceServer {
private:
    crow::SimpleApp app;
    pqxx::connection& db_conn;
    std::shared_ptr<SessionVerifier> session_verifier;
    std::shared_ptr<FinanceService> finance_service;

    bool verify_session(const std::string& session_token, std::string& user_id);

public:
    FinanceServer(pqxx::connection& postgres, sw::redis::Redis& redis);
    void run(int port);
};

#endif // FINANCE_SERVER_H 