#include "connect_redis.h"
#include "redis_set_token.h"
#include <iostream>
#include <chrono>

int main() {
    ConfigRedis config {
        .host = "localhost",
        .port = 6379,
        .password = "",
        .db = 0
    };

    try {
        auto redis = connect_to_redis(config);
        std::cout << "Connected to Redis!\n";

        const std::string test_token = "test_token_123";
        const std::string test_id = "user_42";

        set_token(redis, test_token, test_id);

        auto id = redis.hget(test_token, "id");
        auto expires_at = redis.hget(test_token, "expires_at");

        if (id && expires_at) {
            std::cout << "Data verification:\n"
                      << "ID: " << *id << "\n"
                      << "Expires: " << *expires_at << "\n";
        } else {
            std::cerr << "Data verification failed!\n";
        }

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}