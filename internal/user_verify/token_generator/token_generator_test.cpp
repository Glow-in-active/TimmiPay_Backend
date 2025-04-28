#include <gtest/gtest.h>
#include <sw/redis++/redis++.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "token_generator.h"
#include "uuid_generator.h"
#include "../storage/user_verify/redis_set/redis_set_token.h"

class TokenGeneratorTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        sw::redis::ConnectionOptions opts;
        opts.host = "localhost";
        opts.port = 6379;
        opts.db = 15;
        redis = std::make_unique<sw::redis::Redis>(opts);
    }
    
    static void TearDownTestSuite() {
        redis->flushdb();
    }

    void SetUp() override {
        redis->flushdb();
    }

    static std::unique_ptr<sw::redis::Redis> redis;
    UUIDGenerator uuid_gen; 
};

std::unique_ptr<sw::redis::Redis> TokenGeneratorTest::redis = nullptr;


TEST_F(TokenGeneratorTest, GeneratesValidTokenAndSavesToRedis) {
    TokenGenerator generator(uuid_gen, *redis);
    User test_user{"user123", "test@example.com", "hash"};

    const std::string token = generator.GenerateToken(test_user);
    
    EXPECT_EQ(token.size(), 36);
    EXPECT_EQ(token[8], '-');
    EXPECT_EQ(token[13], '-');
    EXPECT_EQ(token[18], '-');
    EXPECT_EQ(token[23], '-');

    auto stored_id = redis->hget(token, "id");
    ASSERT_TRUE(stored_id);
    EXPECT_EQ(stored_id.value(), test_user.id);

    long long ttl = redis->ttl(token);
    EXPECT_GE(ttl, 590);
    EXPECT_LE(ttl, 600);
}

TEST_F(TokenGeneratorTest, TokenUniqueness) {
    TokenGenerator generator(uuid_gen, *redis);
    User test_user{"user456", "test2@example.com", "hash"};

    const std::string token1 = generator.GenerateToken(test_user);
    const std::string token2 = generator.GenerateToken(test_user);
    
    EXPECT_NE(token1, token2);
    EXPECT_TRUE(redis->hexists(token1, "id"));
    EXPECT_TRUE(redis->hexists(token2, "id"));
}