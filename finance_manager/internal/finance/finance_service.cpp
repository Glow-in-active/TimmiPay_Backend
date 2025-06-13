#include "finance_service.h"
#include <stdexcept>

FinanceService::FinanceService(pqxx::connection& db_conn)
    : db_conn(db_conn) {}

std::vector<std::pair<std::string, double>> FinanceService::get_user_balance(const std::string& user_id) {
    pqxx::work txn(db_conn);
    auto result = txn.exec_params(
        "SELECT a.balance, c.code FROM accounts a "
        "JOIN currencies c ON a.currency_id = c.id "
        "WHERE a.user_id = $1",
        user_id
    );

    std::vector<std::pair<std::string, double>> balances;
    for (const auto& row : result) {
        balances.emplace_back(
            row["code"].as<std::string>(),
            row["balance"].as<double>()
        );
    }

    return balances;
}

std::string FinanceService::transfer_money(
    const std::string& from_user_id,
    const std::string& to_username,
    double amount,
    const std::string& currency_code
) {
    pqxx::transaction tx(db_conn);
    std::string transfer_id;

    try {
        // Получаем ID валюты
        std::string currency_id = get_currency_id(tx, currency_code);
        if (currency_id.empty()) {
            throw std::runtime_error("Invalid currency code");
        }

        // Получаем ID получателя
        std::string to_user_id = get_user_id_by_username(tx, to_username);
        if (to_user_id.empty()) {
            throw std::runtime_error("Recipient not found");
        }

        // Получаем счета
        Account from_account = get_account(tx, from_user_id, currency_id);
        Account to_account = get_account(tx, to_user_id, currency_id);

        // Создаем запись о переводе со статусом 'pending'
        transfer_id = tx.exec_params(
            "INSERT INTO transfers (from_account, to_account, amount, status) "
            "VALUES ($1, $2, $3, 'pending') RETURNING id",
            from_account.id,
            to_account.id,
            amount
        )[0]["id"].as<std::string>();

        // Проверяем баланс
        if (from_account.balance < amount) {
            throw std::runtime_error("Insufficient funds");
        }

        // Обновляем балансы
        update_account_balance(tx, from_account.id, -amount);
        update_account_balance(tx, to_account.id, amount);

        // Обновляем статус перевода на 'completed' после успешного выполнения
        tx.exec_params(
            "UPDATE transfers SET status = 'completed' WHERE id = $1",
            transfer_id
        );

        tx.commit(); // Commit if successful

    } catch (const std::exception& e) {
        if (!transfer_id.empty()) { // Only update if a transfer record was created
            tx.exec_params(
                "UPDATE transfers SET status = 'failed', error_message = $1 WHERE id = $2",
                e.what(),
                transfer_id
            );
            tx.commit(); // Commit the failed status
        } else {
            tx.abort(); // Abort the transaction for early failures (no transfer_id)
        }
        throw; // Re-throw the exception for Crow to handle (e.g., return 500)
    }

    return transfer_id;
}

std::vector<Transfer> FinanceService::get_transaction_history(const std::string& user_id, int page, int limit) {
    pqxx::work txn(db_conn);
    int offset = (page - 1) * limit;
    auto result = txn.exec_params(
        "SELECT t.* FROM transfers t "
        "JOIN accounts a1 ON t.from_account = a1.id "
        "JOIN accounts a2 ON t.to_account = a2.id "
        "WHERE a1.user_id = $1 OR a2.user_id = $1 "
        "ORDER BY t.created_at DESC "
        "LIMIT $2 OFFSET $3",
        user_id,
        limit,
        offset
    );

    std::vector<Transfer> transfers;
    for (const auto& row : result) {
        transfers.push_back(Transfer::from_row(row));
    }

    return transfers;
}

std::string FinanceService::get_currency_id(pqxx::work& txn, const std::string& currency_code) {
    auto result = txn.exec_params(
        "SELECT id FROM currencies WHERE code = $1",
        currency_code
    );

    if (result.empty()) {
        return "";
    }

    return result[0]["id"].as<std::string>();
}

std::string FinanceService::get_user_id_by_username(pqxx::work& txn, const std::string& username) {
    auto result = txn.exec_params(
        "SELECT id FROM users WHERE username = $1",
        username
    );

    if (result.empty()) {
        return "";
    }

    return result[0]["id"].as<std::string>();
}

Account FinanceService::get_account(pqxx::work& txn, const std::string& user_id, const std::string& currency_id) {
    auto result = txn.exec_params(
        "SELECT * FROM accounts WHERE user_id = $1 AND currency_id = $2",
        user_id,
        currency_id
    );

    if (result.empty()) {
        throw std::runtime_error("Account not found");
    }

    return Account::from_row(result[0]);
}

void FinanceService::update_account_balance(pqxx::work& txn, const std::string& account_id, double amount) {
    txn.exec_params(
        "UPDATE accounts SET balance = balance + $1 WHERE id = $2",
        amount,
        account_id
    );
} 