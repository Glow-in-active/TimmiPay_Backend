#pragma once
#include "../../../storage/config/config.h"
#include "../../../storage/redis_config/config_redis.h"
#include <pqxx/pqxx>
#include <sw/redis++/redis++.h>

struct DBConnections {
    pqxx::connection postgres;
    sw::redis::Redis redis;
};

DBConnections initialize_databases();