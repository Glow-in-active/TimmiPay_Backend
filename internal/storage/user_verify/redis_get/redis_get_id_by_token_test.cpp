#include "redis_get_id_by_token.h"
#include "../redis_set/redis_set_token.h"
#include <gtest/gtest.h>
#include <sw/redis++/redis++.h>
#include <memory>
#include <chrono>
#include <thread>
#include <vector>

#include "../../config/config.h"
#include "../../redis_config/config_redis.h"
#include "../../redis_connect/connect_redis.h"

class RedisGetIdByTokenTest : public ::testing::Test {
protected:
    void SetUp() override {
        ConfigRedis redis_config = load_redis_config("database_config/test_redis_config.json");
        
        redis = std::make_unique<sw::redis::Redis>(connect_to_redis(redis_config));
        redis->flushall();

        set_token(*redis, "valid_token_1", "user_100");
        set_token(*redis, "valid_token_2", "user_200");
        
        std::vector<std::pair<sw::redis::StringView, std::string>> fields = {
            {"expires_at", "1234567890"}
        };
        redis->hset("token_without_id", fields.begin(), fields.end());
    }

    void TearDown() override {
        redis->flushall();
    }

    std::unique_ptr<sw::redis::Redis> redis;
};

TEST_F(RedisGetIdByTokenTest, ReturnsCorrectIdForValidTokens) {
    EXPECT_EQ(get_id_by_token(*redis, "valid_token_1"), "user_100");
    EXPECT_EQ(get_id_by_token(*redis, "valid_token_2"), "user_200");
}

TEST_F(RedisGetIdByTokenTest, ThrowsAfterTokenExpiration) {
    set_token(*redis, "expiring_token", "user_300");
    redis->expire("expiring_token", 1);
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    EXPECT_THROW(
        get_id_by_token(*redis, "expiring_token"),
        std::runtime_error
    );
}

TEST_F(RedisGetIdByTokenTest, ThrowsForInvalidToken) {
    EXPECT_THROW(
        get_id_by_token(*redis, "non_existent_token"),
        std::runtime_error
    );
}

TEST_F(RedisGetIdByTokenTest, ThrowsForTokenWithoutIdField) {
    EXPECT_THROW(
        get_id_by_token(*redis, "token_without_id"),
        std::runtime_error
    );
}

TEST_F(RedisGetIdByTokenTest, ErrorMessageContainsCorrectInfo) {
    try {
        get_id_by_token(*redis, "invalid_token_123");
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& e) {
        EXPECT_EQ(
            std::string(e.what()), 
            "System error: Token not found or id field missing"
        );
    }
}

TEST_F(RedisGetIdByTokenTest, ThrowsOnRedisConnectionError) {
    sw::redis::ConnectionOptions opts;
    opts.host = "invalid_host.local";
    opts.port = 1234;
    
    sw::redis::Redis broken_redis(opts);
    
    EXPECT_THROW(
        get_id_by_token(broken_redis, "any_token"),
        std::runtime_error
    );
}

TEST_F(RedisGetIdByTokenTest, HandlesConcurrentAccess) {
    constexpr int thread_count = 10;
    std::vector<std::thread> threads;
    
    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back([this, i]() {
            try {
                auto result = get_id_by_token(*redis, "valid_token_1");
                EXPECT_EQ(result, "user_100");
            } catch (...) {
                FAIL() << "Unexpected exception in thread " << i;
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
}