#pragma once

#include <string>
#include <pqxx/pqxx>

/**
 * @brief Структура, представляющая счет пользователя.
 */
struct Account {
    std::string id;
    std::string user_id;
    std::string currency_id;
    double balance;

    /**
     * @brief Создает объект Account из строки результата запроса pqxx.
     *
     * @param row Объект pqxx::row, содержащий данные счета из базы данных.
     * @return Объект Account, заполненный данными из строки.
     */
    static Account from_row(const pqxx::row& row) {
        Account account;
        account.id = row["id"].as<std::string>();
        account.user_id = row["user_id"].as<std::string>();
        account.currency_id = row["currency_id"].as<std::string>();
        account.balance = row["balance"].as<double>();
        return account;
    }
}; 