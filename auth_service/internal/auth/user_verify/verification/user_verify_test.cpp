#include <gtest/gtest.h>
#include <pqxx/pqxx>
#include "user_verify.h"
#include "../../../../uuid_generator/uuid_generator.h"
#include "../../../models/user.h"
#include <sw/redis++/redis++.h>
#include "../../../../storage/config/config.h"
#include "../../../../storage/redis_config/config_redis.h"
#include "../../../../storage/postgres_connect/connect.h"   
#include "../../../../storage/redis_connect/connect_redis.h"  

const std::string VALID_PASSWORD_HASH = 
    "5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8";

class UserVerifierTest : public ::testing::Test {
protected:
    void SetUp() override {
        try {
            Config postgres_config = load_config("database_config/test_postgres_config.json");
            
            conn = std::make_unique<pqxx::connection>(connect_to_database(postgres_config));

            ConfigRedis redis_config = load_redis_config("database_config/test_redis_config.json");

            redis = std::make_unique<sw::redis::Redis>(connect_to_redis(redis_config));

            uuidGenerator = std::make_unique<UUIDGenerator>();
            
            testUserId = uuidGenerator->generateUUID();
            testEmail = "test_" + testUserId + "@example.com";
            
            insertTestUser();
        } catch (const std::exception& e) {
            FAIL() << "Setup failed: " << e.what();
        }
    }

    void TearDown() override {
        pqxx::work txn(*conn);
        try {
            txn.exec_params(
                "DELETE FROM transfers "
                "WHERE from_account IN (SELECT id FROM accounts WHERE user_id = $1) "
                "OR to_account IN (SELECT id FROM accounts WHERE user_id = $1)",
                testUserId
            );
            
            txn.exec_params(
                "DELETE FROM accounts WHERE user_id = $1",
                testUserId
            );
            
            txn.exec_params(
                "DELETE FROM users WHERE id = $1",
                testUserId
            );
            
            txn.commit();
        } catch (const std::exception& e) {
            txn.abort();
            FAIL() << "Cleanup failed: " << e.what();
        }
    }

    void insertTestUser() {
        pqxx::work txn(*conn);
        try {
            txn.exec_params(
                "INSERT INTO users (id, username, email, password_hash) "
                "VALUES ($1, $2, $3, $4)",
                testUserId,
                "testuser",
                testEmail,
                VALID_PASSWORD_HASH
            );
            txn.commit();
        } catch (const std::exception& e) {
            txn.abort();
            FAIL() << "Failed to insert test user: " << e.what();
        }
    }

    std::unique_ptr<pqxx::connection> conn;
    std::unique_ptr<UUIDGenerator> uuidGenerator;
    std::unique_ptr<sw::redis::Redis> redis;
    std::string testUserId;
    std::string testEmail;
};

TEST_F(UserVerifierTest, ThrowsExceptionForWrongEmail) {
    UserVerifier verifier(*conn, *redis);

    EXPECT_THROW({
        verifier.GenerateToken("wrong_" + testUserId + "@example.com", VALID_PASSWORD_HASH);
    }, std::runtime_error);
}

TEST_F(UserVerifierTest, ThrowsExceptionForWrongPassword) {
    UserVerifier verifier(*conn, *redis);

    EXPECT_THROW({
        verifier.GenerateToken(testEmail, "wrong_hash");
    }, std::runtime_error);
}
