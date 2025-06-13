#pragma once

#include <string>
#include <pqxx/pqxx>

struct Transfer {
    std::string id;
    std::string from_account;
    std::string to_account;
    double amount;
    std::string status;
    std::string error_message;
    std::string created_at;
    std::string updated_at;

    static Transfer from_row(const pqxx::row& row) {
        Transfer transfer;
        transfer.id = row["id"].as<std::string>();
        transfer.from_account = row["from_account"].as<std::string>();
        transfer.to_account = row["to_account"].as<std::string>();
        transfer.amount = row["amount"].as<double>();
        transfer.status = row["status"].as<std::string>();
        if (row["error_message"].is_null()) {
            transfer.error_message = "";
        } else {
            transfer.error_message = row["error_message"].as<std::string>();
        }
        transfer.created_at = row["created_at"].as<std::string>();
        transfer.updated_at = row["updated_at"].as<std::string>();
        return transfer;
    }
}; 