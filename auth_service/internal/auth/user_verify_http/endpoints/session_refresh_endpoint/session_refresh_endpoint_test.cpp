#include <gtest/gtest.h>
#include <crow.h>
#include <nlohmann/json.hpp>
#include "session_refresh_endpoint.h"
#include "../session_hold/session_hold.h"
#include "../../../user_verify/token_generator/token_generator.h"
#include "../../../../../storage/redis_config/config_redis.h"
#include "../../../../../storage/redis_connect/connect_redis.h"
#include "../../../../../storage/user_verify/redis_set/redis_set_token.h"

class SessionHoldAccessor : public SessionHold {
public:
    using SessionHold::SessionHold;

    sw::redis::Redis& get_redis() {
        return this->redis_;
    }
};

class MockSessionHold : public SessionHoldAccessor {
public:
    MockSessionHold(sw::redis::Redis& redis) : SessionHoldAccessor(redis) {}

    nlohmann::json HandleRequest(const nlohmann::json& request_data) {
        std::cout << "Request data in HandleRequest: " << request_data.dump() << std::endl;

        last_request = request_data;

        if (request_data.contains("token")) {
            std::string token = request_data["token"];
            std::cout << "Token from request: " << token << std::endl;
            set_token(this->get_redis(), token, "user_id");
        } else {
            std::cout << "Token not found in request!" << std::endl;
        }

        return response_to_return;
    }

    nlohmann::json last_request;
    nlohmann::json response_to_return;
};

class SessionRefreshEndpointTest : public ::testing::Test {
protected:
    ConfigRedis redis_config = load_redis_config("database_config/test_redis_config.json");
    sw::redis::Redis redis_conn = connect_to_redis(redis_config);

    MockSessionHold handler{redis_conn};
    crow::request req;

    UUIDGenerator uuid_gen;
    TokenGenerator token_gen{uuid_gen, redis_conn};
};

TEST_F(SessionRefreshEndpointTest, HandlesInvalidJson) {
    req.body = "invalid json";

    auto handler_func = create_session_refresh_handler(handler);
    auto response = handler_func(req);

    EXPECT_EQ(response.code, 400);
    auto response_json = nlohmann::json::parse(response.body);
    EXPECT_TRUE(response_json.contains("error"));
    EXPECT_EQ(response_json["error"], "Invalid JSON format");
}

TEST_F(SessionRefreshEndpointTest, HandlesTokenNotFoundOrExpired) {
    req.body = R"({"token": "expired_token"})";
    handler.response_to_return = {{"error", "Token not found or expired"}};

    auto handler_func = create_session_refresh_handler(handler);
    auto response = handler_func(req);

    EXPECT_EQ(response.code, 404);
    auto response_json = nlohmann::json::parse(response.body);
    EXPECT_EQ(response_json["error"], "Token not found or expired");
}

TEST_F(SessionRefreshEndpointTest, HandlesJsonFormatError) {
    req.body = R"({"token": 1234})";
    handler.response_to_return = {
        {"error", "Invalid JSON format"},
        {"details", "Expected string for token"}
    };

    auto handler_func = create_session_refresh_handler(handler);
    auto response = handler_func(req);

    EXPECT_EQ(response.code, 400);
    auto response_json = nlohmann::json::parse(response.body);
    EXPECT_EQ(response_json["error"], "Invalid JSON format");
}
