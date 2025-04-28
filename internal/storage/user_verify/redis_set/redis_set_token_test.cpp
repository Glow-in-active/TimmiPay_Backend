#include "redis_set_token.h"
#include "connect_redis.h"
#include <gtest/gtest.h>
#include <sw/redis++/redis++.h>
#include <chrono>
#include <memory>
#include <thread>

class RedisSetTokenTest : public ::testing::Test {
protected:
    void SetUp() override {
        ConfigRedis test_config{
            .host = "localhost",
            .port = 6379,
            .password = "",
            .db = 15
        };

        redis = std::make_unique<sw::redis::Redis>(connect_to_redis(test_config));
        redis->flushdb();
    }

    void TearDown() override {
        redis->flushdb();
    }

    std::unique_ptr<sw::redis::Redis> redis;
};

TEST_F(RedisSetTokenTest, WritesValidDataWithTTL) {
    const std::string token = "test_token_abc";
    const std::string id = "user_123";
    
    ASSERT_NO_THROW(set_token(*redis, token, id));

    auto stored_id = redis->hget(token, "id");
    auto expires_at = redis->hget(token, "expires_at");
    
    ASSERT_TRUE(static_cast<bool>(stored_id));
    ASSERT_TRUE(static_cast<bool>(expires_at));
    EXPECT_EQ(*stored_id, id);

    auto ttl = redis->ttl(token);
    EXPECT_GT(ttl, 590);
    EXPECT_LE(ttl, 600);
}

TEST_F(RedisSetTokenTest, ExpirationTimeIsCorrect) {
    const std::string token = "test_token_time";
    const std::string id = "user_456";
    
    const auto start = std::chrono::system_clock::now();
    set_token(*redis, token, id);
    const auto end = std::chrono::system_clock::now();

    const auto expires_at = redis->hget(token, "expires_at");
    ASSERT_TRUE(static_cast<bool>(expires_at));
    
    const auto expiration_time = std::chrono::system_clock::time_point(
        std::chrono::seconds(std::stoll(*expires_at))
    );

    const auto min_expected = start + std::chrono::minutes(10) - std::chrono::seconds(5);
    const auto max_expected = end + std::chrono::minutes(10) + std::chrono::seconds(5);
    
    EXPECT_GT(expiration_time, min_expected);
    EXPECT_LT(expiration_time, max_expected);
}

TEST_F(RedisSetTokenTest, OverwritesExistingToken) {
    const std::string token = "test_token_overwrite";
    const std::string id1 = "user_old";
    const std::string id2 = "user_new";
    
    set_token(*redis, token, id1);
    const auto first_ttl = redis->ttl(token);
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    set_token(*redis, token, id2);
    
    const auto stored_id = redis->hget(token, "id");
    const auto new_ttl = redis->ttl(token);
    
    ASSERT_TRUE(static_cast<bool>(stored_id));
    EXPECT_EQ(*stored_id, id2);
    EXPECT_GT(new_ttl, 598);
}

TEST_F(RedisSetTokenTest, DataPersistsUntilExpiration) {
    const std::string token = "test_token_persist";
    const std::string id = "user_789";
    
    set_token(*redis, token, id);
    redis->expire(token, 3);
    
    EXPECT_EQ(redis->exists(token), 1);
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    EXPECT_EQ(redis->exists(token), 1);
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    EXPECT_EQ(redis->exists(token), 0);
}