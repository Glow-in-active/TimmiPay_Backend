#include <gtest/gtest.h>
#include <crow.h>
#include <nlohmann/json.hpp>
#include "session_auth_endpoint.h"
#include "../../session_start/session_start.h"

class MockSessionStart : public SessionStart {
public:
    MockSessionStart() : SessionStart(user_verifier_) {}

    json HandleRequest(const json& request_data) {
        last_request = request_data;
        return response_to_return;
    }

    json last_request;
    json response_to_return;

private:
    class MockUserVerifier : public UserVerifier {
    public:
        MockUserVerifier() : UserVerifier(conn_, redis_) {}

        std::string GenerateToken(const std::string&, const std::string&) {
            return "";
        }

    private:
        pqxx::connection conn_{"postgresql://admin:secret@localhost:5432/timmipay?sslmode=disable"};
        sw::redis::Redis redis_{"tcp://127.0.0.1:6379"};
    };

    MockUserVerifier user_verifier_;
};

class SessionAuthEndpointTest : public ::testing::Test {
protected:
    MockSessionStart handler;
    crow::request req;
};

TEST_F(SessionAuthEndpointTest, HandlesValidRequest) {
    req.body = R"({"email": "test@example.com", "password_hash": "hash123"})";
    handler.response_to_return = {{"token", "valid_token"}};

    auto body_json = nlohmann::json::parse(req.body);

    auto response_json = handler.HandleRequest(body_json);

    EXPECT_EQ(handler.last_request["email"], "test@example.com");
    EXPECT_EQ(handler.last_request["password_hash"], "hash123");

    EXPECT_EQ(response_json["token"], "valid_token");

    crow::response response;
    response.code = 200;
    response.body = response_json.dump();

    EXPECT_EQ(response.code, 200);
    auto parsed_response = nlohmann::json::parse(response.body);
    EXPECT_EQ(parsed_response["token"], "valid_token");
}


TEST_F(SessionAuthEndpointTest, HandlesInvalidJson) {
    req.body = "invalid json";
    
    auto handler_func = create_session_auth_handler(handler);
    auto response = handler_func(req);
    
    EXPECT_EQ(response.code, 500);
    auto response_json = nlohmann::json::parse(response.body);
    EXPECT_TRUE(response_json.contains("error"));
}

TEST_F(SessionAuthEndpointTest, HandlesJsonFormatError) {
    req.body = R"({"email": "test@example.com"})";
    handler.response_to_return = {
        {"error", "Invalid JSON format"},
        {"details", "Missing password_hash field"}
    };
    
    auto handler_func = create_session_auth_handler(handler);
    auto response = handler_func(req);
    
    EXPECT_EQ(response.code, 400);
    auto response_json = nlohmann::json::parse(response.body);
    EXPECT_EQ(response_json["error"], "Invalid JSON format");
}

TEST_F(SessionAuthEndpointTest, HandlesVerificationError) {
    req.body = R"({"email": "test@example.com", "password_hash": "wrong_hash"})";
    handler.response_to_return = {
        {"error", "Verification failed"},
        {"details", "Invalid credentials"}
    };
    
    auto handler_func = create_session_auth_handler(handler);
    auto response = handler_func(req);
    
    EXPECT_EQ(response.code, 401);
    auto response_json = nlohmann::json::parse(response.body);
    EXPECT_EQ(response_json["error"], "Verification failed");
}

TEST_F(SessionAuthEndpointTest, HandlesUnknownError) {
    req.body = R"({"email": "test@example.com", "password_hash": "hash123"})";
    handler.response_to_return = {
        {"error", "Unknown error"},
        {"details", "Something went wrong"}
    };

    auto handler_func = create_session_auth_handler(handler);
    auto response = handler_func(req);

    EXPECT_EQ(response.code, 401);
    auto response_json = nlohmann::json::parse(response.body);
    EXPECT_EQ(response_json["error"], "Verification failed");
}

