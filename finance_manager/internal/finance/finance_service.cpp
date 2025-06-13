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
    pqxx::work txn(db_conn);

    // Получаем ID валюты
    std::string currency_id = get_currency_id(txn, currency_code);
    if (currency_id.empty()) {
        throw std::runtime_error("Invalid currency code");
    }

    // Получаем ID получателя
    std::string to_user_id = get_user_id_by_username(txn, to_username);
    if (to_user_id.empty()) {
        throw std::runtime_error("Recipient not found");
    }

    // Получаем счета
    Account from_account = get_account(txn, from_user_id, currency_id);
    Account to_account = get_account(txn, to_user_id, currency_id);

    // Создаем запись о переводе со статусом 'pending'
    auto transfer_id = txn.exec_params(
        "INSERT INTO transfers (from_account, to_account, amount, status) "
        "VALUES ($1, $2, $3, 'pending') RETURNING id",
        from_account.id,
        to_account.id,
        amount
    )[0]["id"].as<std::string>();

    try {
        // Проверяем баланс
        if (from_account.balance < amount) {
            throw std::runtime_error("Insufficient funds");
        }

        // Обновляем балансы
        update_account_balance(txn, from_account.id, -amount);
        update_account_balance(txn, to_account.id, amount);

        // Обновляем статус перевода на 'completed' после успешного выполнения
        txn.exec_params(
            "UPDATE transfers SET status = 'completed' WHERE id = $1",
            transfer_id
        );

    } catch (const std::runtime_error& e) {
        // Обновляем статус перевода на 'failed' и записываем ошибку
        txn.exec_params(
            "UPDATE transfers SET status = 'failed', error_message = $1 WHERE id = $2",
            e.what(),
            transfer_id
        );
        throw; // Перебрасываем исключение, чтобы Crow обрабатывал ошибку
    } catch (const std::exception& e) {
        // Ловим другие возможные исключения и также устанавливаем статус 'failed'
        txn.exec_params(
            "UPDATE transfers SET status = 'failed', error_message = $1 WHERE id = $2",
            e.what(),
            transfer_id
        );
        throw; // Перебрасываем исключение
    }

    txn.commit();
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