#include <gtest/gtest.h>
#include "config_redis.h"
#include <fstream>
#include <nlohmann/json.hpp>

TEST(RedisConfigTest, LoadsCorrectConfig) {
    const std::string filename = "test_redis_config.json";
    {
        std::ofstream file(filename);
        file << R"({
            "host": "redis.example.com",
            "port": 6379,
            "password": "secret123",
            "db": 5
        })";
    }

    ConfigRedis config = load_redis_config(filename);
    
    EXPECT_EQ(config.host, "redis.example.com");
    EXPECT_EQ(config.port, 6379);
    EXPECT_EQ(config.password, "secret123");
    EXPECT_EQ(config.db, 5);
    
    std::remove(filename.c_str());
}

TEST(RedisConfigTest, ThrowsOnMissingFile) {
    EXPECT_THROW(
        { load_redis_config("non_existent_redis_config.json"); },
        std::runtime_error
    );
}

TEST(RedisConfigTest, ThrowsOnMissingField) {
    const std::string filename = "missing_redis_field.json";
    {
        std::ofstream file(filename);
        file << R"({
            "host": "localhost",
            "port": 6379,
            "password": ""
        })";
    }

    EXPECT_THROW(
        { load_redis_config(filename); },
        nlohmann::json::exception
    );
    
    std::remove(filename.c_str());
}

TEST(RedisConfigTest, ThrowsOnInvalidType) {
    const std::string filename = "invalid_redis_type.json";
    {
        std::ofstream file(filename);
        file << R"({
            "host": "localhost",
            "port": "6379",
            "password": "",
            "db": 0
        })";
    }

    EXPECT_THROW(
        { load_redis_config(filename); },
        nlohmann::json::exception
    );
    
    std::remove(filename.c_str());
}

TEST(RedisConfigTest, ThrowsOnMalformedJSON) {
    const std::string filename = "malformed_redis.json";
    {
        std::ofstream file(filename);
        file << "{ invalid_json }";
    }

    EXPECT_THROW(
        { load_redis_config(filename); },
        nlohmann::json::parse_error
    );
    
    std::remove(filename.c_str());
}

TEST(RedisConfigTest, ThrowsOnEmptyJSON) {
    const std::string filename = "empty_redis.json";
    {
        std::ofstream file(filename);
        file << "";
    }

    EXPECT_THROW(
        { load_redis_config(filename); },
        nlohmann::json::parse_error
    );
    
    std::remove(filename.c_str());
}