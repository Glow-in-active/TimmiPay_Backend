#pragma once

#include <string>
#include <pqxx/pqxx>

struct Currency {
    std::string id;
    std::string code;
    std::string name;

    static Currency from_row(const pqxx::row& row) {
        Currency currency;
        currency.id = row["id"].as<std::string>();
        currency.code = row["code"].as<std::string>();
        currency.name = row["name"].as<std::string>();
        return currency;
    }
}; 