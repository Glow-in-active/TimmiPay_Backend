#pragma once

#include <memory>
#include <optional>
#include <pqxx/pqxx>
#include <string>
#include <vector>

#include "../models/account.h"
#include "../models/currency.h"
#include "../models/transfer.h"

/**
 * @brief Класс для предоставления финансовых услуг, таких как получение
 * баланса, перевод денег и история транзакций.
 */
class FinanceService {
 public:
  /**
   * @brief Конструктор для FinanceService.
   *
   * Инициализирует FinanceService с предоставленным соединением с базой данных
   * PostgreSQL.
   *
   * @param db_conn Ссылка на объект pqxx::connection, используемый для
   * взаимодействия с базой данных.
   */
  explicit FinanceService(pqxx::connection& db_conn);

  /**
   * @brief Получает баланс пользователя для каждой валюты.
   *
   * @param user_id Уникальный идентификатор пользователя.
   * @return Вектор пар, где каждая пара содержит код валюты (string) и баланс
   * (double).
   */
  std::vector<std::pair<std::string, double>> get_user_balance(
      const std::string& user_id);

  /**
   * @brief Осуществляет перевод денег между пользователями.
   *
   * @param from_user_id ID пользователя-отправителя.
   * @param to_username Имя пользователя-получателя.
   * @param amount Сумма перевода.
   * @param currency Код валюты перевода (например, "USD", "EUR").
   * @return ID созданной транзакции.
   */
  std::string transfer_money(const std::string& from_user_id,
                             const std::string& to_username, double amount,
                             const std::string& currency);

  /**
   * @brief Получает историю транзакций для указанного пользователя.
   *
   * @param user_id Уникальный идентификатор пользователя.
   * @param page Номер страницы для пагинации (начиная с 1).
   * @param limit Максимальное количество записей на одной странице.
   * @return Вектор объектов Transfer, представляющих историю транзакций
   * пользователя.
   */
  std::vector<Transfer> get_transaction_history(const std::string& user_id,
                                                int page, int limit);

 private:
  pqxx::connection& db_conn;

  /**
   * @brief Получает ID валюты по ее коду.
   *
   * @param txn Ссылка на активную транзакцию `pqxx::work`.
   * @param currency_code Трехбуквенный код валюты.
   * @return ID валюты в виде строки, или пустая строка, если валюта не найдена.
   */
  std::string get_currency_id(pqxx::work& txn,
                              const std::string& currency_code);

  /**
   * @brief Получает ID пользователя по его имени пользователя.
   *
   * @param txn Ссылка на активную транзакцию `pqxx::work`.
   * @param username Имя пользователя.
   * @return ID пользователя в виде строки, или пустая строка, если пользователь
   * не найден.
   */
  std::string get_user_id_by_username(pqxx::work& txn,
                                      const std::string& username);

  /**
   * @brief Получает объект счета пользователя по ID пользователя и ID валюты.
   *
   * @param txn Ссылка на активную транзакцию `pqxx::work`.
   * @param user_id ID пользователя, которому принадлежит счет.
   * @param currency_id ID валюты счета.
   * @return `std::optional<Account>` содержащий объект Account, если счет
   * найден, иначе `std::nullopt`.
   */
  std::optional<Account> get_account(pqxx::work& txn,
                                     const std::string& user_id,
                                     const std::string& currency_id);

  /**
   * @brief Получает ID счета пользователя по ID пользователя и ID валюты.
   *
   * @param txn Ссылка на активную транзакцию `pqxx::work`.
   * @param user_id ID пользователя, которому принадлежит счет.
   * @param currency_id ID валюты счета.
   * @return `std::optional<std::string>` содержащий ID счета в виде строки,
   * если счет найден, иначе `std::nullopt`.
   */
  std::optional<std::string> get_account_id(pqxx::work& txn,
                                            const std::string& user_id,
                                            const std::string& currency_id);

  /**
   * @brief Обновляет баланс счета.
   *
   * @param txn Ссылка на активную транзакцию `pqxx::work`.
   * @param account_id ID счета, баланс которого нужно обновить.
   * @param amount Сумма, на которую нужно изменить баланс (положительное для
   * увеличения, отрицательное для уменьшения).
   */
  void update_account_balance(pqxx::work& txn, const std::string& account_id,
                              double amount);
};