#include "finance_service.h"
#include <stdexcept>

/**
 * @brief Конструктор для FinanceService.
 *
 * Инициализирует FinanceService с предоставленным соединением с базой данных PostgreSQL.
 *
 * @param db_conn Ссылка на объект pqxx::connection, используемый для взаимодействия с базой данных.
 */
FinanceService::FinanceService(pqxx::connection& db_conn)
    : db_conn(db_conn) {}

/**
 * @brief Получает баланс пользователя для каждой валюты.
 *
 * Выполняет запрос к базе данных для получения балансов всех счетов, принадлежащих указанному пользователю,
 * и возвращает их вместе с соответствующим кодом валюты.
 *
 * @param user_id Уникальный идентификатор пользователя.
 * @return Вектор пар, где каждая пара содержит код валюты (string) и баланс (double).
 */
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

/**
 * @brief Осуществляет перевод денег между пользователями.
 *
 * Выполняет атомарную операцию перевода, обновляя балансы счетов отправителя и получателя,
 * и записывает транзакцию в базу данных.
 *
 * @param from_user_id ID пользователя-отправителя.
 * @param to_username Имя пользователя-получателя.
 * @param amount Сумма перевода.
 * @param currency_code Код валюты перевода (например, "USD", "EUR").
 * @return ID созданной транзакции.
 * @throws std::runtime_error В случае неверного кода валюты, отсутствия получателя,
 *                             отсутствия счета, недостаточных средств или других ошибок базы данных.
 */
std::string FinanceService::transfer_money(
    const std::string& from_user_id,
    const std::string& to_username,
    double amount,
    const std::string& currency_code
) {
    pqxx::transaction tx(db_conn);
    std::string transfer_id;
    std::string error_message;

    try {
        // Step 1: Preliminary checks and get IDs for initial pending transaction
        std::string currency_id = get_currency_id(tx, currency_code);
        if (currency_id.empty()) {
            error_message = "Invalid currency code.";
            throw std::runtime_error(error_message);
        }

        std::string to_user_id = get_user_id_by_username(tx, to_username);
        if (to_user_id.empty()) {
            error_message = "Recipient not found.";
            throw std::runtime_error(error_message);
        }

        std::optional<std::string> from_account_id_opt = get_account_id(tx, from_user_id, currency_id);
        if (!from_account_id_opt) {
            error_message = "Sender account not found for this currency.";
            throw std::runtime_error(error_message);
        }
        std::string from_account_id = *from_account_id_opt;

        std::optional<std::string> to_account_id_opt = get_account_id(tx, to_user_id, currency_id);
        if (!to_account_id_opt) {
            error_message = "Recipient account not found for this currency.";
            throw std::runtime_error(error_message);
        }
        std::string to_account_id = *to_account_id_opt;

        // Step 2: Insert transaction with 'pending' status immediately
        transfer_id = tx.exec_params(
            "INSERT INTO transfers (from_account, to_account, amount, status) "
            "VALUES ($1, $2, $3, 'pending') RETURNING id",
            from_account_id,
            to_account_id,
            amount
        )[0]["id"].as<std::string>();

        // Step 3: Perform detailed checks and update transaction status
        // Retrieve full Account objects to check balances (these can throw if accounts are not found, but we already have their IDs)
        Account from_account = get_account(tx, from_user_id, currency_id).value(); 
        Account to_account = get_account(tx, to_user_id, currency_id).value();

        if (from_account.balance < amount) {
            error_message = "Insufficient funds.";
            throw std::runtime_error(error_message);
        }

        update_account_balance(tx, from_account.id, -amount);
        update_account_balance(tx, to_account.id, amount);

        tx.exec_params(
            "UPDATE transfers SET status = 'completed' WHERE id = $1",
            transfer_id
        );

        tx.commit();

    } catch (const std::exception& e) {
        if (!transfer_id.empty()) {
            // Update the status to 'failed' only if a transfer record was created
            tx.exec_params(
                "UPDATE transfers SET status = 'failed', error_message = $1 WHERE id = $2",
                e.what(),
                transfer_id
            );
            tx.commit(); // Commit the failed status
        } else {
            tx.abort(); // Abort the transaction for early failures (no transfer_id, e.g. invalid currency)
        }
        throw; // Re-throw the exception for Crow to handle
    }

    return transfer_id;
}

/**
 * @brief Получает историю транзакций для указанного пользователя.
 *
 * Выполняет запрос к базе данных для получения списка транзакций, в которых участвовал пользователь,
 * с возможностью пагинации.
 *
 * @param user_id Уникальный идентификатор пользователя.
 * @param page Номер страницы для пагинации (начиная с 1).
 * @param limit Максимальное количество записей на одной странице.
 * @return Вектор объектов Transfer, представляющих историю транзакций пользователя.
 */
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
    /**
     * @brief Получает ID валюты по ее коду.
     *
     * Выполняет запрос к базе данных для поиска ID валюты по ее трехбуквенному коду (например, "USD").
     *
     * @param txn Ссылка на активную транзакцию `pqxx::work`.
     * @param currency_code Трехбуквенный код валюты.
     * @return ID валюты в виде строки, или пустая строка, если валюта не найдена.
     */
    auto result = txn.exec_params(
        "SELECT id FROM currencies WHERE code = $1",
        currency_code
    );

    if (result.empty()) {
        return "";
    }

    return result[0]["id"].as<std::string>();
}

/**
 * @brief Получает ID пользователя по его имени пользователя.
 *
 * Выполняет запрос к базе данных для поиска ID пользователя по его имени пользователя.
 *
 * @param txn Ссылка на активную транзакцию `pqxx::work`.
 * @param username Имя пользователя.
 * @return ID пользователя в виде строки, или пустая строка, если пользователь не найден.
 */
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

/**
 * @brief Получает объект счета пользователя по ID пользователя и ID валюты.
 *
 * Выполняет запрос к базе данных для получения полной информации о счете.
 *
 * @param txn Ссылка на активную транзакцию `pqxx::work`.
 * @param user_id ID пользователя, которому принадлежит счет.
 * @param currency_id ID валюты счета.
 * @return `std::optional<Account>` содержащий объект Account, если счет найден, иначе `std::nullopt`.
 */
std::optional<Account> FinanceService::get_account(pqxx::work& txn, const std::string& user_id, const std::string& currency_id) {
    auto result = txn.exec_params(
        "SELECT * FROM accounts WHERE user_id = $1 AND currency_id = $2",
        user_id,
        currency_id
    );

    if (result.empty()) {
        return std::nullopt; // Return nullopt if account not found
    }

    return Account::from_row(result[0]);
}

/**
 * @brief Получает ID счета пользователя по ID пользователя и ID валюты.
 *
 * Выполняет запрос к базе данных для поиска ID счета пользователя.
 *
 * @param txn Ссылка на активную транзакцию `pqxx::work`.
 * @param user_id ID пользователя, которому принадлежит счет.
 * @param currency_id ID валюты счета.
 * @return `std::optional<std::string>` содержащий ID счета в виде строки, если счет найден, иначе `std::nullopt`.
 */
std::optional<std::string> FinanceService::get_account_id(pqxx::work& txn, const std::string& user_id, const std::string& currency_id) {
    auto result = txn.exec_params(
        "SELECT id FROM accounts WHERE user_id = $1 AND currency_id = $2",
        user_id,
        currency_id
    );

    if (result.empty()) {
        return std::nullopt; // Return nullopt if account ID not found
    }

    return result[0]["id"].as<std::string>();
}

/**
 * @brief Обновляет баланс счета.
 *
 * Выполняет обновление баланса указанного счета в базе данных.
 *
 * @param txn Ссылка на активную транзакцию `pqxx::work`.
 * @param account_id ID счета, баланс которого нужно обновить.
 * @param amount Сумма, на которую нужно изменить баланс (положительное для увеличения, отрицательное для уменьшения).
 */
void FinanceService::update_account_balance(pqxx::work& txn, const std::string& account_id, double amount) {
    txn.exec_params(
        "UPDATE accounts SET balance = balance + $1 WHERE id = $2",
        amount,
        account_id
    );
} 