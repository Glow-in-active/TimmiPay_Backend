#include "../internal/storage/redis_connect/connect_redis.h"
#include <iostream>

int main() {
    try {
        ConfigRedis config = load_redis_config("redis_config.json");
        auto redis = connect_to_redis(config);

        auto pong = redis.ping();
        if (pong != "PONG") {
            throw std::runtime_error("Unexpected ping response: " + pong);
        }

        redis.set("test", "value");
        auto value = redis.get("test");
        
        if (value) {
            std::cout << "Success! Value: " << *value << std::endl;
        } else {
            std::cout << "Key not found" << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Redis error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
