// internal/storage/user_verify/user_verify_test.cpp
#include <gtest/gtest.h>
#include <pqxx/pqxx>
#include "user_verify.h"
#include "../../models/user.h"

constexpr auto TEST_UUID = "f715b468-50ab-4b2b-be3d-e6f1c84b8f1d";
const std::string DB_CONN_STR = 
    "postgresql://admin:secret@localhost:5432/timmipay?sslmode=disable";

const std::string VALID_PASSWORD_HASH = 
    "5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8";

class UserVerifierTest : public ::testing::Test {
protected:
    void SetUp() override {
        try {
            conn = std::make_unique<pqxx::connection>(DB_CONN_STR);
            prepareTestData();
        } catch (const std::exception& e) {
            FAIL() << "Ошибка подключения к БД: " << e.what();
        }
    }

    void TearDown() override {
        pqxx::work txn(*conn);
        txn.exec("DELETE FROM users");
        txn.commit();
    }

    void prepareTestData() {
        pqxx::work txn(*conn);
        
        txn.exec(
            "CREATE TABLE IF NOT EXISTS users ("
            "id UUID PRIMARY KEY, "
            "email TEXT UNIQUE NOT NULL, "
            "password_hash TEXT NOT NULL)"
        );

        txn.exec_params(
            "INSERT INTO users (id, email, password_hash) "
            "VALUES ($1, $2, $3) "
            "ON CONFLICT (email) DO NOTHING",
            TEST_UUID,
            "valid@example.com",
            VALID_PASSWORD_HASH
        );
        txn.commit();
    }

    std::unique_ptr<pqxx::connection> conn;
};

TEST_F(UserVerifierTest, GeneratesValidTokenForCorrectCredentials) {
    UserVerifier verifier(*conn);
    
    std::string token = verifier.GenerateToken(
        "valid@example.com",
        VALID_PASSWORD_HASH
    );
    
    EXPECT_FALSE(token.empty()) << "Токен не должен быть пустым при верных данных";
    EXPECT_EQ(token.size(), 64) << "Ожидается 64-символьный hex-токен";
}

TEST_F(UserVerifierTest, ReturnsEmptyTokenForWrongEmail) {
    UserVerifier verifier(*conn);
    
    std::string token = verifier.GenerateToken(
        "wrong@example.com",
        VALID_PASSWORD_HASH
    );
    
    EXPECT_TRUE(token.empty()) << "Токен должен быть пустым при неверном email";
}

TEST_F(UserVerifierTest, ReturnsEmptyTokenForWrongPassword) {
    UserVerifier verifier(*conn);
    
    std::string token = verifier.GenerateToken(
        "valid@example.com",
        "неправильный_хэш"
    );
    
    EXPECT_TRUE(token.empty()) << "Токен должен быть пустым при неверном пароле";
}