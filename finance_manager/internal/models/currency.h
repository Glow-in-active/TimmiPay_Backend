#pragma once

#include <pqxx/pqxx>
#include <string>

/**
 * @brief Структура, представляющая валюту.
 */
struct Currency {
  std::string id;
  std::string code;
  std::string name;

  /**
   * @brief Создает объект Currency из строки результата запроса pqxx.
   *
   * @param row Объект pqxx::row, содержащий данные валюты из базы данных.
   * @return Объект Currency, заполненный данными из строки.
   */
  static Currency from_row(const pqxx::row& row) {
    Currency currency;
    currency.id = row["id"].as<std::string>();
    currency.code = row["code"].as<std::string>();
    currency.name = row["name"].as<std::string>();
    return currency;
  }
};