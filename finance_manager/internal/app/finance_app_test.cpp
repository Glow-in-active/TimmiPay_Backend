#include <gtest/gtest.h>
#include "finance_service.h"
#include "../../../../storage/config/config.h"
#include "../../../../storage/postgres_connect/connect.h"
#include "../../../../uuid_generator/uuid_generator.h"

/**
 * @brief Тестовый класс для FinanceService, использующий реальное соединение с базой данных.
 *
 * Настраивает и очищает тестовые данные в базе данных для каждого теста.
 */
class FinanceServiceTest : public ::testing::Test {
protected:
    std::string test_user1_id;
    std::string test_user2_id;
    std::string test_usd_id;
    std::string test_eur_id;
    UUIDGenerator uuid_gen;

    /**
     * @brief Настраивает тестовую среду перед каждым тестом.
     *
     * Инициализирует соединение с базой данных, создает FinanceService
     * и настраивает тестовые данные.
     */
    void SetUp() override {
        Config config = load_config("database_config/test_postgres_config.json");
        db_conn = std::make_unique<pqxx::connection>(connect_to_database(config));
        finance_service = std::make_unique<FinanceService>(*db_conn);
        
        setupTestData();
    }

    /**
     * @brief Очищает тестовую среду после каждого теста.
     *
     * Удаляет тестовые данные из базы данных.
     */
    void TearDown() override {
        cleanupTestData();
    }

    /**
     * @brief Настраивает тестовые данные в базе данных.
     *
     * Вставляет тестовых пользователей, валюты и счета для использования в тестах.
     */
    void setupTestData() {
        pqxx::work txn(*db_conn);
        
        // Create test users
        test_user1_id = uuid_gen.generateUUID();
        test_user2_id = uuid_gen.generateUUID();
        txn.exec_params("INSERT INTO users (id, username, email, password_hash) VALUES ($1, 'test_user1', 'test1@example.com', 'password_hash_1')", test_user1_id);
        txn.exec_params("INSERT INTO users (id, username, email, password_hash) VALUES ($1, 'test_user2', 'test2@example.com', 'password_hash_2')", test_user2_id);
        
        // Create test currencies
        test_usd_id = uuid_gen.generateUUID();
        test_eur_id = uuid_gen.generateUUID();
        txn.exec_params("INSERT INTO currencies (id, code, name) VALUES ($1, 'USD', 'United States Dollar')", test_usd_id);
        txn.exec_params("INSERT INTO currencies (id, code, name) VALUES ($1, 'EUR', 'Euro')", test_eur_id);
        
        // Create test accounts
        txn.exec_params("INSERT INTO accounts (id, user_id, currency_id, balance) VALUES ($1, $2, $3, 1000.0)", uuid_gen.generateUUID(), test_user1_id, test_usd_id);
        txn.exec_params("INSERT INTO accounts (id, user_id, currency_id, balance) VALUES ($1, $2, $3, 500.0)", uuid_gen.generateUUID(), test_user2_id, test_usd_id);
        
        txn.commit();
    }

    /**
     * @brief Очищает тестовые данные из базы данных.
     *
     * Удаляет все записи из таблиц transfers, accounts, currencies и users.
     */
    void cleanupTestData() {
        pqxx::work txn(*db_conn);
        txn.exec("DELETE FROM transfers");
        txn.exec("DELETE FROM accounts");
        txn.exec("DELETE FROM currencies");
        txn.exec("DELETE FROM users");
        txn.commit();
    }

    std::unique_ptr<pqxx::connection> db_conn;
    std::unique_ptr<FinanceService> finance_service;
};

/**
 * @brief Проверяет получение баланса пользователя.
 *
 * Тест получает баланс для тестового пользователя и проверяет количество счетов,
 * код валюты и сумму баланса.
 */
TEST_F(FinanceServiceTest, GetUserBalance) {
    auto balances = finance_service->get_user_balance(test_user1_id);
    ASSERT_EQ(balances.size(), 1);
    EXPECT_EQ(balances[0].first, "USD");
    EXPECT_DOUBLE_EQ(balances[0].second, 1000.0);
}

/**
 * @brief Проверяет успешную передачу денег между пользователями.
 *
 * Тест выполняет перевод денег и проверяет, что ID перевода не пуст,
 * а балансы отправителя и получателя обновлены корректно.
 */
TEST_F(FinanceServiceTest, TransferMoneySuccess) {
    std::string transfer_id = finance_service->transfer_money(test_user1_id, "test_user2", 100.0, "USD");
    EXPECT_FALSE(transfer_id.empty());

    auto sender_balances = finance_service->get_user_balance(test_user1_id);
    auto receiver_balances = finance_service->get_user_balance(test_user2_id);
    
    EXPECT_DOUBLE_EQ(sender_balances[0].second, 900.0);
    EXPECT_DOUBLE_EQ(receiver_balances[0].second, 600.0);
}

/**
 * @brief Проверяет, что при недостаточном балансе выбрасывается исключение.
 *
 * Тест пытается выполнить перевод суммы, превышающей доступный баланс пользователя,
 * и ожидает исключения `std::runtime_error`.
 */
TEST_F(FinanceServiceTest, TransferMoneyInsufficientFunds) {
    EXPECT_THROW({
        finance_service->transfer_money(test_user1_id, "test_user2", 2000.0, "USD");
    }, std::runtime_error);
}

/**
 * @brief Проверяет, что при неверной валюте выбрасывается исключение.
 *
 * Тест пытается выполнить перевод с использованием несуществующей валюты
 * и ожидает исключения `std::runtime_error`.
 */
TEST_F(FinanceServiceTest, TransferMoneyInvalidCurrency) {
    EXPECT_THROW({
        finance_service->transfer_money(test_user1_id, "test_user2", 100.0, "INVALID");
    }, std::runtime_error);
}

/**
 * @brief Проверяет получение истории транзакций пользователя.
 *
 * Тест выполняет перевод, затем получает историю транзакций для отправителя и получателя,
 * проверяя размер истории, сумму транзакции и ее статус.
 */
TEST_F(FinanceServiceTest, GetTransactionHistory) {
    finance_service->transfer_money(test_user1_id, "test_user2", 100.0, "USD");
    
    auto sender_history = finance_service->get_transaction_history(test_user1_id, 1, 10);
    ASSERT_EQ(sender_history.size(), 1);
    EXPECT_DOUBLE_EQ(sender_history[0].amount, 100.0);
    EXPECT_EQ(sender_history[0].status, "completed");
    
    auto receiver_history = finance_service->get_transaction_history(test_user2_id, 1, 10);
    ASSERT_EQ(receiver_history.size(), 1);
    EXPECT_DOUBLE_EQ(receiver_history[0].amount, 100.0);
    EXPECT_EQ(receiver_history[0].status, "completed");
} 