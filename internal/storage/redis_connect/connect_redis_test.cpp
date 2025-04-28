#include "connect_redis.h"
#include <gtest/gtest.h>
#include <sw/redis++/redis++.h>

class ConnectRedisTest : public ::testing::Test {
protected:
    void SetUp() override {
        valid_config.host = "localhost";
        valid_config.port = 6379;
        valid_config.password = "";
        valid_config.db = 0;
    }

    ConfigRedis valid_config;
};

TEST_F(ConnectRedisTest, ConnectsWithValidConfig) {
    EXPECT_NO_THROW({
        auto redis = connect_to_redis(valid_config);
        redis.ping();
    });
}

