#include <gtest/gtest.h>

#include "../config/config.h"
#include "../postgres_connect/connect.h"

class DatabaseConnectionIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        valid_config = load_config("database_config/test_postgres_config.json");
    }

    Config valid_config;
};

TEST_F(DatabaseConnectionIntegrationTest, ConnectsSuccessfully) {
    EXPECT_NO_THROW({
        auto conn = connect_to_database(valid_config);
        EXPECT_TRUE(conn.is_open());
    });
}

TEST_F(DatabaseConnectionIntegrationTest, ThrowsOnInvalidHost) {
    Config config = valid_config;
    config.host = "invalid_host";

    EXPECT_THROW({
        auto conn = connect_to_database(config);
    }, std::runtime_error);
}

TEST_F(DatabaseConnectionIntegrationTest, ThrowsOnInvalidPort) {
    Config config = valid_config;
    config.port = 9999;

    EXPECT_THROW({
        auto conn = connect_to_database(config);
    }, std::runtime_error);
}

TEST_F(DatabaseConnectionIntegrationTest, ThrowsOnInvalidCredentials) {
    Config config = valid_config;
    config.user = "wrong_user";
    config.password = "wrong_password";

    EXPECT_THROW({
        auto conn = connect_to_database(config);
    }, std::runtime_error);
}

TEST_F(DatabaseConnectionIntegrationTest, ThrowsOnInvalidDatabaseName) {
    Config config = valid_config;
    config.dbname = "nonexistent_db";

    EXPECT_THROW({
        auto conn = connect_to_database(config);
    }, std::runtime_error);
}
