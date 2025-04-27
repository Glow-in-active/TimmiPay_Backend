#include <gtest/gtest.h>
#include "../config/config.h"
#include "connect.h"

TEST(DatabaseConnectionIntegrationTest, ConnectsSuccessfully) {
    Config config {
        "localhost",
        5432,
        "admin",
        "secret",
        "timmipay",
        "disable"
    };

    EXPECT_NO_THROW({
        auto conn = connect_to_database(config);
        EXPECT_TRUE(conn.is_open());
    });
}

TEST(DatabaseConnectionIntegrationTest, ThrowsOnInvalidHost) {
    Config config {
        "invalid_host",
        5432,
        "admin",
        "secret",
        "timmipay",
        "disable"
    };

    EXPECT_THROW({
        auto conn = connect_to_database(config);
    }, std::runtime_error);
}

TEST(DatabaseConnectionIntegrationTest, ThrowsOnInvalidPort) {
    Config config {
        "localhost",
        9999,  // неверный порт
        "admin",
        "secret",
        "timmipay",
        "disable"
    };

    EXPECT_THROW({
        auto conn = connect_to_database(config);
    }, std::runtime_error);
}

TEST(DatabaseConnectionIntegrationTest, ThrowsOnInvalidCredentials) {
    Config config {
        "localhost",
        5432,
        "wrong_user",
        "wrong_password",
        "timmipay",
        "disable"
    };

    EXPECT_THROW({
        auto conn = connect_to_database(config);
    }, std::runtime_error);
}

TEST(DatabaseConnectionIntegrationTest, ThrowsOnInvalidDatabaseName) {
    Config config {
        "localhost",
        5432,
        "admin",
        "secret",
        "nonexistent_db",
        "disable"
    };

    EXPECT_THROW({
        auto conn = connect_to_database(config);
    }, std::runtime_error);
}
