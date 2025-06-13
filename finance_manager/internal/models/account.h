#pragma once

#include <string>
#include <pqxx/pqxx>

struct Account {
    std::string id;
    std::string user_id;
    std::string currency_id;
    double balance;

    static Account from_row(const pqxx::row& row) {
        Account account;
        account.id = row["id"].as<std::string>();
        account.user_id = row["user_id"].as<std::string>();
        account.currency_id = row["currency_id"].as<std::string>();
        account.balance = row["balance"].as<double>();
        return account;
    }
}; 