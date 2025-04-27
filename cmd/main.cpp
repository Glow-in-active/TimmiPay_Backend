#include "../internal/uuid_generator/uuid_generator.h"
#include <iostream>
#include "../internal/storage/redis_config/config_redis.h"
int main() {
    try {
        ConfigRedis redis_cfg = load_redis_config("../redis_config.json");
        std::cout << "Connecting to Redis: " 
                  << redis_cfg.host << ":" << redis_cfg.port 
                  << " (DB: " << redis_cfg.db << ")";
    } 
    catch (const std::exception& e) {
        std::cout << "Config error: " << e.what();
    }
    return 0;
}