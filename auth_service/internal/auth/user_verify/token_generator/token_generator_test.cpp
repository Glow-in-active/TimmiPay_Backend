#include <gtest/gtest.h>
#include <sw/redis++/redis++.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "token_generator.h"
#include "uuid_generator.h"
#include "../../../../storage/user_verify/redis_set/redis_set_token.h"
#include "../../../../storage/redis_config/config_redis.h"
#include "../../../../storage/redis_connect/connect_redis.h"

/**
 * @brief Тестовый класс для TokenGenerator.
 *
 * Настраивает и очищает тестовую среду Redis для каждого тестового набора.
 */
class TokenGeneratorTest : public ::testing::Test {
protected:
    /**
     * @brief Настраивает тестовую среду перед выполнением всех тестов.
     *
     * Загружает конфигурацию Redis, устанавливает соединение и очищает базу данных Redis.
     */
    static void SetUpTestSuite() {
        ConfigRedis redis_config = load_redis_config("database_config/test_redis_config.json");
        
        redis = std::make_unique<sw::redis::Redis>(connect_to_redis(redis_config));
        
        redis->flushdb();
    }

    /**
     * @brief Очищает тестовую среду после выполнения всех тестов.
     *
     * Очищает базу данных Redis и освобождает ресурсы.
     */
    static void TearDownTestSuite() {
        redis->flushdb();
        redis.reset();
    }

    /**
     * @brief Настраивает тестовую среду перед каждым тестом.
     *
     * Очищает базу данных Redis, чтобы каждый тест начинался с чистого состояния.
     */
    void SetUp() override {
        redis->flushdb();
    }

    static std::unique_ptr<sw::redis::Redis> redis;
    UUIDGenerator uuid_gen;
};

std::unique_ptr<sw::redis::Redis> TokenGeneratorTest::redis = nullptr;

/**
 * @brief Проверяет, что TokenGenerator генерирует валидный токен и сохраняет его в Redis.
 *
 * Тест проверяет формат сгенерированного токена (наличие дефисов, длину) и убеждается,
 * что токен успешно сохраняется в Redis с правильным ID пользователя и TTL.
 */
TEST_F(TokenGeneratorTest, GeneratesValidTokenAndSavesToRedis) {
    TokenGenerator generator(uuid_gen, *redis);
    User test_user{"user123", "test@example.com", "hash"};

    const std::string token = generator.GenerateToken(test_user);

    EXPECT_EQ(token.size(), 36);
    EXPECT_EQ(token[8], '-');
    EXPECT_EQ(token[13], '-');
    EXPECT_EQ(token[18], '-');
    EXPECT_EQ(token[23], '-');

    EXPECT_TRUE(redis->exists(token));

    auto stored_id = redis->hget(token, "id");
    ASSERT_TRUE(stored_id) << "Значение id должно быть в Redis";
    EXPECT_EQ(*stored_id, test_user.id);

    long long ttl = redis->ttl(token);
    EXPECT_GE(ttl, 0);
}

/**
 * @brief Проверяет уникальность сгенерированных токенов.
 *
 * Тест генерирует два токена для одного и того же пользователя и проверяет,
 * что они отличаются, а также оба присутствуют в Redis.
 */
TEST_F(TokenGeneratorTest, TokenUniqueness) {
    TokenGenerator generator(uuid_gen, *redis);
    User test_user{"user456", "test2@example.com", "hash"};

    const std::string token1 = generator.GenerateToken(test_user);
    const std::string token2 = generator.GenerateToken(test_user);

    EXPECT_NE(token1, token2);

    EXPECT_TRUE(redis->exists(token1));
    EXPECT_TRUE(redis->exists(token2));
}
