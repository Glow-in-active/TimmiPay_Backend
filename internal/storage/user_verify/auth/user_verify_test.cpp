#include <gtest/gtest.h>
#include <pqxx/pqxx>
#include "user_verify.h"
#include "../../../uuid_generator/uuid_generator.h"

class UserStorageProdTest : public ::testing::Test {
protected:
    pqxx::connection* conn;
    std::string test_user_id;
    std::string test_email;
    UUIDGenerator uuid_gen;

    void SetUp() override {
        test_user_id = uuid_gen.generateUUID();
        test_email = "test_" + test_user_id + "@example.com";

        conn = new pqxx::connection(
            "host=localhost "
            "port=5432 "
            "user=admin "
            "password=secret "
            "dbname=timmipay "
            "sslmode=disable"
        );
        
        pqxx::work setup_work(*conn);
        setup_work.exec_params(
            "INSERT INTO users (id, username, email, password_hash) "
            "VALUES ($1, $2, $3, $4)",
            test_user_id,
            "test_user",
            test_email,
            "test_hash"
        );
        setup_work.commit();
    }

    void TearDown() override {
        pqxx::work teardown_work(*conn);
        teardown_work.exec_params(
            "DELETE FROM users WHERE id = $1",
            test_user_id
        );
        teardown_work.commit();

        delete conn;
    }
};

TEST_F(UserStorageProdTest, CreatedUserExists) {
    UserStorage storage(*conn);
    User user = storage.GetUserByEmail(test_email);
    
    ASSERT_FALSE(user.id.empty());
    EXPECT_EQ(user.id, test_user_id);
    EXPECT_EQ(user.email, test_email);
}

TEST_F(UserStorageProdTest, PasswordVerificationWorks) {
    UserStorage storage(*conn);
    User user = storage.GetUserByEmail(test_email);
    
    EXPECT_TRUE(storage.VerifyPassword(user, "test_hash"));
    EXPECT_FALSE(storage.VerifyPassword(user, "wrong_hash"));
}