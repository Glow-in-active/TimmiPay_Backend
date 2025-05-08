#include <gtest/gtest.h>
#include <pqxx/pqxx>
#include "users_balance.h"
#include "../../uuid_generator/uuid_generator.h"
#include "../config/config.h"
#include "../postgres_connect/connect.h"

class BalanceStorageProdTest : public ::testing::Test {
protected:
    pqxx::connection* conn;
    std::string test_user_id;
    std::string usd_currency_id;
    std::string eur_currency_id;
    UUIDGenerator uuid_gen;

    void SetUp() override {
        Config config = load_config("database_config/test_postgres_config.json");
        conn = new pqxx::connection(connect_to_database(config));
        test_user_id = uuid_gen.generateUUID();

        pqxx::work setup_work(*conn);
        
        setup_work.exec_params(
            "INSERT INTO users (id, username, email, password_hash) "
            "VALUES ($1, $2, $3, $4) "
            "ON CONFLICT (id) DO NOTHING",
            test_user_id,
            "balance_test_user",
            "balance_test@example.com",
            "test_hash"
        );

        setup_work.exec(
            "INSERT INTO currencies (id, code, name) "
            "VALUES "
            "(gen_random_uuid(), 'USD', 'US Dollar'), "
            "(gen_random_uuid(), 'EUR', 'Euro') "
            "ON CONFLICT (code) DO NOTHING"
        );

        pqxx::result currency_res = setup_work.exec(
            "SELECT id, code FROM currencies "
            "WHERE code IN ('USD', 'EUR')"
        );

        for (const auto& row : currency_res) {
            if (row["code"].as<std::string>() == "USD") {
                usd_currency_id = row["id"].as<std::string>();
            }
            else if (row["code"].as<std::string>() == "EUR") {
                eur_currency_id = row["id"].as<std::string>();
            }
        }

        setup_work.exec_params(
            "INSERT INTO accounts (user_id, currency_id, balance) "
            "VALUES ($1, $2, 1000.50), ($1, $3, 500.75) "
            "ON CONFLICT (user_id, currency_id) DO UPDATE SET "
            "balance = EXCLUDED.balance",
            test_user_id,
            usd_currency_id,
            eur_currency_id
        );

        setup_work.commit();
    }

    void TearDown() override {
        pqxx::work teardown_work(*conn);
        
        teardown_work.exec_params(
            "DELETE FROM accounts WHERE user_id = $1",
            test_user_id
        );

        teardown_work.exec_params(
            "DELETE FROM users WHERE id = $1",
            test_user_id
        );

        teardown_work.commit();
        delete conn;
    }
};

TEST_F(BalanceStorageProdTest, ReturnsCorrectBalances) {
    BalanceStorage storage(*conn);
    auto balances = storage.GetUserBalances(test_user_id);

    ASSERT_EQ(balances.size(), 2);
    EXPECT_DOUBLE_EQ(balances["USD"], 1000.50);
    EXPECT_DOUBLE_EQ(balances["EUR"], 500.75);
}

TEST_F(BalanceStorageProdTest, ReturnsEmptyForInvalidUser) {
    BalanceStorage storage(*conn);
    
    std::string invalid_uuid = uuid_gen.generateUUID();
    
    {
        pqxx::work w(*conn);
        w.exec_params0("DELETE FROM users WHERE id = $1", invalid_uuid);
        w.commit();
    }

    auto balances = storage.GetUserBalances(invalid_uuid);
    
    EXPECT_TRUE(balances.empty());
}

TEST_F(BalanceStorageProdTest, HandlesUserWithNoAccounts) {
    pqxx::work work(*conn);
    work.exec_params(
        "DELETE FROM accounts WHERE user_id = $1",
        test_user_id
    );
    work.commit();

    BalanceStorage storage(*conn);
    auto balances = storage.GetUserBalances(test_user_id);
    
    EXPECT_TRUE(balances.empty());
}