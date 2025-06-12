#include <gtest/gtest.h>
#include <crow.h>
#include "dependencies.h"
#include "../db_init/db_init.h"
#include "../../auth/user_verify_http/endpoints/session_refresh_endpoint/session_refresh_endpoint.h"
#include "../../auth/user_verify_http/endpoints/session_auth_endpoint/session_auth_endpoint.h"
#include "../../../storage/config/config.h"
#include "../../../storage/redis_config/config_redis.h"
#include "../../../storage/postgres_connect/connect.h"
#include "../../../storage/redis_connect/connect_redis.h"

static DBConnections initialize_test_databases_for_dependencies() {
    Config postgres_config = load_config("database_config/test_postgres_config.json");
    ConfigRedis redis_config = load_redis_config("database_config/test_redis_config.json");

    pqxx::connection postgres_conn = connect_to_database(postgres_config);
    sw::redis::Redis redis_conn = connect_to_redis(redis_config);

    return {
        std::move(postgres_conn),
        std::move(redis_conn)
    };
}

TEST(DependenciesTest, InitializeDependenciesDoesNotThrow) {
    DBConnections db = initialize_test_databases_for_dependencies();
    EXPECT_NO_THROW({
        Dependencies deps = initialize_dependencies(db);
    });
}

TEST(DependenciesTest, HandlersAreValid) {
    DBConnections db = initialize_test_databases_for_dependencies();
    Dependencies deps = initialize_dependencies(db);

    EXPECT_TRUE(&deps.user_verifier != nullptr);
    EXPECT_TRUE(&deps.session_start_handler != nullptr);
    EXPECT_TRUE(&deps.session_hold_handler != nullptr);
}