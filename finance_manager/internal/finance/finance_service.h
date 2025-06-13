#pragma once

#include <memory>
#include <string>
#include <vector>
#include <pqxx/pqxx>
#include "../models/account.h"
#include "../models/transfer.h"
#include "../models/currency.h"

class FinanceService {
public:
    explicit FinanceService(pqxx::connection& db_conn);

    // Получение баланса пользователя
    std::vector<std::pair<std::string, double>> get_user_balance(const std::string& user_id);

    // Перевод денег
    std::string transfer_money(
        const std::string& from_user_id,
        const std::string& to_username,
        double amount,
        const std::string& currency
    );

    // Получение истории транзакций
    std::vector<Transfer> get_transaction_history(const std::string& user_id, int page, int limit);

private:
    pqxx::connection& db_conn;

    // Вспомогательные методы
    std::string get_currency_id(pqxx::work& txn, const std::string& currency_code);
    std::string get_user_id_by_username(pqxx::work& txn, const std::string& username);
    Account get_account(pqxx::work& txn, const std::string& user_id, const std::string& currency_id);
    void update_account_balance(pqxx::work& txn, const std::string& account_id, double amount);
}; 