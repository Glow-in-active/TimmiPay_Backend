#include <gtest/gtest.h>
#include <sw/redis++/redis++.h>
#include "../config/config.h"
#include "../redis_config/config_redis.h"
#include "../postgres_connect/connect.h"
#include "../redis_connect/connect_redis.h"

class ConnectRedisTest : public ::testing::Test {
protected:
    void SetUp() override {
        redis_config = load_redis_config("database_config/test_redis_config.json");
    }

    ConfigRedis redis_config;
};

TEST_F(ConnectRedisTest, ConnectsWithValidConfig) {
    EXPECT_NO_THROW({
        auto redis = connect_to_redis(redis_config);
        redis.ping();
    });
}
