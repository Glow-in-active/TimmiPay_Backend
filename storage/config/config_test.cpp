#include <gtest/gtest.h>
#include "config.h"
#include <fstream>
#include <nlohmann/json.hpp>

TEST(ConfigTest, LoadsCorrectConfig) {
    const std::string filename = "test_config.json";
    {
        std::ofstream file(filename);
        file << R"({
            "host": "localhost",
            "port": 5432,
            "user": "postgres",
            "password": "secret123",
            "dbname": "mydatabase",
            "sslmode": "verify-full"
        })";
    }

    Config config = load_config(filename);
    
    EXPECT_EQ(config.host, "localhost");
    EXPECT_EQ(config.port, 5432);
    EXPECT_EQ(config.user, "postgres");
    EXPECT_EQ(config.password, "secret123");
    EXPECT_EQ(config.dbname, "mydatabase");
    EXPECT_EQ(config.sslmode, "verify-full");
    
    std::remove(filename.c_str());
}

TEST(ConfigTest, ThrowsOnMissingFile) {
    EXPECT_THROW(
        { load_config("non_existent_config.json"); },
        std::runtime_error
    );
}

TEST(ConfigTest, ThrowsOnMissingField) {
    const std::string filename = "missing_field.json";
    {
        std::ofstream file(filename);
        file << R"({
            "host": "localhost",
            "port": 5432,
            "user": "postgres",
            "password": "secret123",
            "dbname": "mydatabase"
        })";
    }

    EXPECT_THROW(
        { load_config(filename); },
        nlohmann::json::exception
    );
    
    std::remove(filename.c_str());
}

TEST(ConfigTest, ThrowsOnInvalidType) {
    const std::string filename = "invalid_type.json";
    {
        std::ofstream file(filename);
        file << R"({
            "host": "localhost",
            "port": "5432",
            "user": "postgres",
            "password": "secret123",
            "dbname": "mydatabase",
            "sslmode": "verify-full"
        })";
    }

    EXPECT_THROW(
        { load_config(filename); },
        nlohmann::json::exception
    );
    
    std::remove(filename.c_str());
}

TEST(ConfigTest, ThrowsOnMalformedJSON) {
    const std::string filename = "malformed.json";
    {
        std::ofstream file(filename);
        file << "{ invalid_json }";
    }

    EXPECT_THROW(
        { load_config(filename); },
        nlohmann::json::parse_error
    );
    
    std::remove(filename.c_str());
}

TEST(ConfigTest, ThrowsOnEmptyJSON) {
    const std::string filename = "empty.json";
    {
        std::ofstream file(filename);
        file << "";
    }

    EXPECT_THROW(
        { load_config(filename); },
        nlohmann::json::parse_error
    );
    
    std::remove(filename.c_str());
}