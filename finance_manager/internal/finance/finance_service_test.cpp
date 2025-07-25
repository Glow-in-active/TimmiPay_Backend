#include "finance_service.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <map>
#include <optional>
#include <pqxx/pqxx>
#include <stdexcept>

#include "../../../storage/config/config.h"
#include "../../../storage/postgres_connect/connect.h"
#include "../../../uuid_generator/uuid_generator.h"
#include "../models/account.h"
#include "../models/transfer.h"

// Global UUID generator for test data
UUIDGenerator uuidGenerator;

class FinanceServiceTest : public ::testing::Test {
 protected:
  std::unique_ptr<pqxx::connection> conn;
  FinanceService* financeService;

  // Test data
  std::string testUser1Id;
  std::string testUser1Username = "testuser1";
  std::string testUser2Id;
  std::string testUser2Username = "testuser2";
  std::string currencyUSDId;
  std::string currencyEURId;
  std::string testUser1AccountUSDId;
  std::string testUser2AccountUSDId;
  std::string testUser1AccountEURId;

  void SetUp() override {
    try {
      Config postgres_config =
          load_config("database_config/test_postgres_config.json");
      conn = std::make_unique<pqxx::connection>(
          connect_to_database(postgres_config));
      financeService = new FinanceService(*conn);

      ClearTestDatabase();

      InsertCurrency("USD", "US Dollar");
      InsertCurrency("EUR", "Euro");
      currencyUSDId = GetCurrencyId("USD");
      currencyEURId = GetCurrencyId("EUR");

      testUser1Id = uuidGenerator.generateUUID();
      InsertUser(testUser1Id, testUser1Username, "user1@example.com",
                 "password_hash_1");
      testUser2Id = uuidGenerator.generateUUID();
      InsertUser(testUser2Id, testUser2Username, "user2@example.com",
                 "password_hash_2");

      testUser1AccountUSDId = uuidGenerator.generateUUID();
      InsertAccount(testUser1AccountUSDId, testUser1Id, currencyUSDId, 1000.0);
      testUser2AccountUSDId = uuidGenerator.generateUUID();
      InsertAccount(testUser2AccountUSDId, testUser2Id, currencyUSDId, 500.0);
      testUser1AccountEURId = uuidGenerator.generateUUID();
      InsertAccount(testUser1AccountEURId, testUser1Id, currencyEURId, 200.0);

    } catch (const std::exception& e) {
      FAIL() << "Setup failed: " << e.what();
    }
  }

  void TearDown() override {
    ClearTestDatabase();
    delete financeService;
  }

  void ClearTestDatabase() {
    pqxx::work txn(*conn);
    txn.exec("DELETE FROM transfers CASCADE;");
    txn.exec("DELETE FROM accounts CASCADE;");
    txn.exec("DELETE FROM users CASCADE;");
    txn.exec("DELETE FROM currencies CASCADE;");
    txn.commit();
  }

  /**
   * @brief Вставляет нового тестового пользователя в базу данных.
   * @param id Уникальный ID пользователя.
   * @param username Имя пользователя.
   * @param email Адрес электронной почты пользователя.
   * @param password_hash Хеш пароля пользователя.
   */
  void InsertUser(const std::string& id, const std::string& username,
                  const std::string& email, const std::string& password_hash) {
    pqxx::work txn(*conn);
    txn.exec_params(
        "INSERT INTO users (id, username, email, password_hash) VALUES ($1, "
        "$2, $3, $4)",
        id, username, email, password_hash);
    txn.commit();
  }

  /**
   * @brief Вставляет новую тестовую валюту в базу данных.
   * @param code Трехбуквенный код валюты (например, "USD").
   * @param name Полное название валюты (например, "United States Dollar").
   */
  void InsertCurrency(const std::string& code, const std::string& name) {
    pqxx::work txn(*conn);
    txn.exec_params(
        "INSERT INTO currencies (id, code, name) VALUES ($1, $2, $3)",
        uuidGenerator.generateUUID(), code, name);
    txn.commit();
  }

  /**
   * @brief Получает ID валюты по ее коду.
   * @param code Трехбуквенный код валюты.
   * @return ID валюты в виде строки, или пустая строка, если валюта не найдена.
   */
  std::string GetCurrencyId(const std::string& code) {
    pqxx::work txn(*conn);
    pqxx::result result =
        txn.exec_params("SELECT id FROM currencies WHERE code = $1", code);
    if (result.empty()) return "";
    return result[0]["id"].as<std::string>();
  }

  /**
   * @brief Вставляет новый тестовый счет в базу данных.
   * @param id Уникальный ID счета.
   * @param user_id ID пользователя, которому принадлежит счет.
   * @param currency_id ID валюты счета.
   * @param balance Начальный баланс счета.
   */
  void InsertAccount(const std::string& id, const std::string& user_id,
                     const std::string& currency_id, double balance) {
    pqxx::work txn(*conn);
    txn.exec_params(
        "INSERT INTO accounts (id, user_id, currency_id, balance) VALUES ($1, "
        "$2, $3, $4)",
        id, user_id, currency_id, balance);
    txn.commit();
  }

  /**
   * @brief Получает объект счета из базы данных по ID пользователя и ID валюты.
   * @param user_id ID пользователя, которому принадлежит счет.
   * @param currency_id ID валюты счета.
   * @return Объект Account, содержащий данные счета.
   */
  Account GetAccountFromDb(const std::string& user_id,
                           const std::string& currency_id) {
    pqxx::work txn(*conn);
    pqxx::result result = txn.exec_params(
        "SELECT * FROM accounts WHERE user_id = $1 AND currency_id = $2",
        user_id, currency_id);
    EXPECT_FALSE(result.empty());
    return Account::from_row(result[0]);
  }
};

/**
 * @brief Проверяет, что `GetUserBalance` возвращает корректные данные.
 *
 * Тест получает балансы для тестового пользователя и проверяет, что количество
 * счетов, коды валют и суммы балансов соответствуют ожидаемым. Также
 * проверяется случай, когда у пользователя нет счетов.
 */
TEST_F(FinanceServiceTest, GetUserBalanceReturnsCorrectData) {
  auto balances = financeService->get_user_balance(testUser1Id);
  ASSERT_EQ(balances.size(), 2);

  std::map<std::string, double> balanceMap;
  for (const auto& p : balances) {
    balanceMap[p.first] = p.second;
  }

  EXPECT_EQ(balanceMap["USD"], 1000.0);
  EXPECT_EQ(balanceMap["EUR"], 200.0);

  auto balancesEmpty =
      financeService->get_user_balance(uuidGenerator.generateUUID());
  EXPECT_TRUE(balancesEmpty.empty());
}

/**
 * @brief Проверяет успешный перевод денег между пользователями.
 *
 * Тест выполняет перевод денег и проверяет, что ID перевода не пуст,
 * а балансы отправителя и получателя обновлены корректно. Также проверяет
 * статус транзакции в базе данных.
 */
TEST_F(FinanceServiceTest, TransferMoneySuccessful) {
  double initialSenderBalance =
      GetAccountFromDb(testUser1Id, currencyUSDId).balance;
  double initialReceiverBalance =
      GetAccountFromDb(testUser2Id, currencyUSDId).balance;
  double transferAmount = 100.0;

  std::string transferId = financeService->transfer_money(
      testUser1Id, testUser2Username, transferAmount, "USD");

  EXPECT_FALSE(transferId.empty());

  double finalSenderBalance =
      GetAccountFromDb(testUser1Id, currencyUSDId).balance;
  double finalReceiverBalance =
      GetAccountFromDb(testUser2Id, currencyUSDId).balance;

  EXPECT_EQ(finalSenderBalance, initialSenderBalance - transferAmount);
  EXPECT_EQ(finalReceiverBalance, initialReceiverBalance + transferAmount);

  pqxx::work txn(*conn);
  pqxx::result result =
      txn.exec_params("SELECT status FROM transfers WHERE id = $1", transferId);
  EXPECT_EQ(result[0]["status"].as<std::string>(), "completed");
}

/**
 * @brief Проверяет, что при недостаточном балансе выбрасывается исключение.
 *
 * Тест пытается выполнить перевод суммы, превышающей доступный баланс
 * пользователя, и ожидает исключения `std::runtime_error`. Также проверяет, что
 * балансы не изменились и была создана запись о неудачном переводе.
 */
TEST_F(FinanceServiceTest, TransferMoneyInsufficientFunds) {
  double transferAmount = 2000.0;  // More than testUser1 has in USD
  EXPECT_THROW(financeService->transfer_money(testUser1Id, testUser2Username,
                                              transferAmount, "USD"),
               std::runtime_error);

  // Verify balances did not change
  EXPECT_EQ(GetAccountFromDb(testUser1Id, currencyUSDId).balance, 1000.0);
  EXPECT_EQ(GetAccountFromDb(testUser2Id, currencyUSDId).balance, 500.0);

  pqxx::work txn(*conn);
  pqxx::result result = txn.exec_params(
      "SELECT COUNT(*) FROM transfers WHERE status = 'failed' AND from_account "
      "IN "
      "(SELECT id FROM accounts WHERE user_id = $1 AND currency_id = $2)",
      testUser1Id, currencyUSDId);
  EXPECT_GT(result[0][0].as<long>(), 0);
}

/**
 * @brief Проверяет, что при неверной валюте выбрасывается исключение.
 *
 * Тест пытается выполнить перевод с использованием несуществующей валюты
 * и ожидает исключения `std::runtime_error`. Также проверяет, что балансы не
 * изменились.
 */
TEST_F(FinanceServiceTest, TransferMoneyInvalidCurrency) {
  EXPECT_THROW(
      financeService->transfer_money(testUser1Id, testUser2Username, 10.0,
                                     "XYZ"  // Invalid currency
                                     ),
      std::runtime_error);

  // Verify no changes to balances
  EXPECT_EQ(GetAccountFromDb(testUser1Id, currencyUSDId).balance, 1000.0);
  EXPECT_EQ(GetAccountFromDb(testUser2Id, currencyUSDId).balance, 500.0);
}

/**
 * @brief Проверяет, что при отсутствии получателя выбрасывается исключение.
 *
 * Тест пытается выполнить перевод несуществующему пользователю
 * и ожидает исключения `std::runtime_error`. Также проверяет, что баланс
 * отправителя не изменился.
 */
TEST_F(FinanceServiceTest, TransferMoneyRecipientNotFound) {
  EXPECT_THROW(financeService->transfer_money(testUser1Id, "non_existent_user",
                                              10.0, "USD"),
               std::runtime_error);
  EXPECT_EQ(GetAccountFromDb(testUser1Id, currencyUSDId).balance, 1000.0);
}

/**
 * @brief Проверяет, что при отсутствии счета отправителя для указанной валюты
 * выбрасывается исключение.
 *
 * Тест пытается выполнить перевод от пользователя, у которого нет счета в
 * указанной валюте, и ожидает исключения `std::runtime_error`. Также проверяет,
 * что баланс отправителя не изменился.
 */
TEST_F(FinanceServiceTest, TransferMoneySenderAccountNotFoundForCurrency) {
  // Attempt transfer from testUser2 (who only has USD) in EUR
  EXPECT_THROW(financeService->transfer_money(testUser2Id, testUser1Username,
                                              10.0, "EUR"),
               std::runtime_error);
  EXPECT_EQ(GetAccountFromDb(testUser2Id, currencyUSDId).balance, 500.0);
}

/**
 * @brief Проверяет, что `GetTransactionHistory` возвращает корректные данные.
 *
 * Тест выполняет несколько переводов, затем получает историю транзакций для
 * отправителя и получателя, проверяя количество транзакций, суммы и статусы.
 */
TEST_F(FinanceServiceTest, GetTransactionHistoryReturnsCorrectData) {
  // Perform some transfers to populate history
  financeService->transfer_money(testUser1Id, testUser2Username, 10.0, "USD");
  financeService->transfer_money(testUser2Id, testUser1Username, 5.0, "USD");
  financeService->transfer_money(testUser1Id, testUser2Username, 20.0, "USD");

  auto history = financeService->get_transaction_history(testUser1Id, 1, 10);
  ASSERT_GE(history.size(), 3);  // May include previous failed transfers if any

  // Verify order (most recent first) and amounts
  EXPECT_EQ(history[0].amount, 20.0);
  EXPECT_EQ(history[1].amount, 5.0);
  EXPECT_EQ(history[2].amount, 10.0);

  // Get history for receiver
  auto receiver_history =
      financeService->get_transaction_history(testUser2Id, 1, 10);
  ASSERT_GE(receiver_history.size(), 3);
  EXPECT_EQ(receiver_history[0].amount, 20.0);
  EXPECT_EQ(receiver_history[1].amount, 5.0);
  EXPECT_EQ(receiver_history[2].amount, 10.0);
}

/**
 * @brief Проверяет пагинацию истории транзакций.
 *
 * Тест выполняет множество переводов и затем проверяет корректность пагинации
 * при получении истории транзакций.
 */
TEST_F(FinanceServiceTest, GetTransactionHistoryPagination) {
  // Create more transactions for pagination test
  for (int i = 0; i < 15; ++i) {
    financeService->transfer_money(testUser1Id, testUser2Username, 1.0, "USD");
  }

  // Get first page (limit 10)
  auto page1 = financeService->get_transaction_history(testUser1Id, 1, 10);
  ASSERT_EQ(page1.size(), 10);

  // Get second page (limit 10)
  auto page2 = financeService->get_transaction_history(testUser1Id, 2, 10);
  ASSERT_EQ(page2.size(),
            8);  // 3 initial + 15 new = 18 total. First page 10, second page 8.

  // Verify that the last element of page1 is the first element of page2 + 10
  // elements in between
  EXPECT_DOUBLE_EQ(
      page1[9].amount,
      page2[0].amount);  // This assumes consistent ordering for simplicity
}