#include "redis_set_token.h"
#include <gtest/gtest.h>
#include <sw/redis++/redis++.h>
#include <chrono>
#include <memory>
#include <thread>

#include "../../config/config.h"
#include "../../redis_config/config_redis.h"
#include "../../postgres_connect/connect.h"
#include "../../redis_connect/connect_redis.h"

/**
 * @brief Тестовый класс для функций установки и продления токенов Redis.
 *
 * Настраивает соединение с Redis и очищает базу данных перед каждым тестом.
 */
class RedisSetTokenTest : public ::testing::Test {
protected:
    /**
     * @brief Настраивает тестовую среду перед каждым тестом.
     *
     * Инициализирует соединение с Redis и очищает базу данных Redis.
     */
    void SetUp() override {
        ConfigRedis redis_config = load_redis_config("database_config/test_redis_config.json");

        redis = std::make_unique<sw::redis::Redis>(connect_to_redis(redis_config));
        redis->flushdb();
    }

    /**
     * @brief Очищает тестовую среду после каждого теста.
     *
     * Очищает базу данных Redis.
     */
    void TearDown() override {
        redis->flushdb();
    }

    std::unique_ptr<sw::redis::Redis> redis;
};

/**
 * @brief Проверяет запись валидных данных с TTL.
 *
 * Тест устанавливает токен в Redis, затем проверяет, что ID пользователя и время истечения срока действия сохранены
 * правильно, а TTL находится в ожидаемом диапазоне.
 */
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

/**
 * @brief Проверяет корректность времени истечения срока действия.
 *
 * Тест устанавливает токен и проверяет, что рассчитанное время истечения срока действия находится
 * в пределах ожидаемого диапазона относительно времени установки.
 */
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

/**
 * @brief Проверяет перезапись существующего токена.
 *
 * Тест устанавливает токен с одним ID, затем перезаписывает его другим ID и проверяет, что ID обновлен
 * и TTL также обновлен.
 */
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

/**
 * @brief Проверяет сохранение данных до истечения срока действия.
 *
 * Тест устанавливает токен с коротким TTL и проверяет, что токен существует до истечения срока действия
 * и удаляется после него.
 */
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
