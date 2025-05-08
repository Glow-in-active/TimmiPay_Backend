#include "users_balance.h"
#include <pqxx/pqxx>
#include <iostream>

BalanceStorage::BalanceStorage(pqxx::connection& conn) : conn_(conn) {}

std::unordered_map<std::string, double> BalanceStorage::GetUserBalances(const std::string& user_id) {
    std::unordered_map<std::string, double> balances;
    
    try {
        pqxx::work txn(conn_);
        pqxx::result res = txn.exec_params(
            "SELECT c.code, a.balance::float "
            "FROM accounts a "
            "JOIN currencies c ON a.currency_id = c.id "
            "WHERE a.user_id = $1",
            user_id
        );

        for (const auto& row : res) {
            balances[row[0].as<std::string>()] = row[1].as<double>();
        }
        txn.commit();
    }
    catch (const std::exception& e) {
        std::cerr << "Balance query error: " << e.what() << std::endl;
    }
    
    return balances;
}