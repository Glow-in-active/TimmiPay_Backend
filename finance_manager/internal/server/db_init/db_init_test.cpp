#include <gtest/gtest.h>
#include "db_init.h"
#include "../../../../storage/config/config.h"
#include "../../../../storage/redis_config/config_redis.h"
#include "../../../../storage/postgres_connect/connect.h"
#include "../../../../storage/redis_connect/connect_redis.h"

DBConnections initialize_finance_test_databases() {
    try {
        Config postgres_config = load_config("database_config/test_postgres_config.json");
        ConfigRedis redis_config = load_redis_config("database_config/test_redis_config.json");

        pqxx::connection postgres_conn = connect_to_database(postgres_config);
        sw::redis::Redis redis_conn = connect_to_redis(redis_config);

        return {
            std::move(postgres_conn),
            std::move(redis_conn)
        };

    } catch (const std::exception& e) {
        throw std::runtime_error(
            "Test database initialization failed: " + std::string(e.what())
        );
    }
}

TEST(FinanceDBInitTest, TestPostgresConnectionIsOpen) {
    DBConnections db = initialize_finance_test_databases();
    EXPECT_TRUE(db.postgres.is_open());
}

TEST(FinanceDBInitTest, TestRedisConnectionIsAlive) {
    DBConnections db = initialize_finance_test_databases();
    EXPECT_NO_THROW({
        std::string pong = db.redis.ping();
        EXPECT_EQ(pong, "PONG");
    });
}

TEST(FinanceDBInitTest, TestInvalidPostgresConfig) {
    Config invalid_config;
    invalid_config.host = "invalid_host";
    invalid_config.port = 12345;
    
    EXPECT_THROW({
        pqxx::connection conn = connect_to_database(invalid_config);
    }, std::runtime_error);
}

TEST(FinanceDBInitTest, TestInvalidRedisConfig) {
    ConfigRedis invalid_config;
    invalid_config.host = "invalid_host";
    invalid_config.port = 12345;
    
    EXPECT_THROW({
        sw::redis::Redis conn = connect_to_redis(invalid_config);
    }, std::runtime_error);
} 