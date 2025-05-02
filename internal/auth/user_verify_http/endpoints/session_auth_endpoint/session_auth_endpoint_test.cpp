#include <gtest/gtest.h>
#include "session_auth_endpoint.h"
#include "../session_start/session_start.h"
#include <nlohmann/json.hpp>

struct FakeConnection {};
struct FakeRedis {};

class DummyUserVerifier : public UserVerifier {
public:
    DummyUserVerifier() 
        : UserVerifier(
            *reinterpret_cast<pqxx::connection*>(&fake_conn_),
            *reinterpret_cast<sw::redis::Redis*>(&fake_redis_)
        ) 
    {}
    
    bool Verify(const std::string& email, const std::string& password_hash) override {
        return false;
    }

private:
    FakeConnection fake_conn_;
    FakeRedis fake_redis_;
};

class ManualMockSessionStart : public SessionStart {
public:
    ManualMockSessionStart() : SessionStart(verifier_) {}
    
    nlohmann::json HandleRequest(const nlohmann::json& input) override {
        last_received_ = input;
        return response_;
    }

    nlohmann::json response_;
    nlohmann::json last_received_;

private:
    DummyUserVerifier verifier_;
};

class SessionAuthEndpointTest : public ::testing::Test {
protected:
    void SetUp() override {
        mock_handler_ = std::make_unique<ManualMockSessionStart>();
        handler_ = create_session_auth_handler(*mock_handler_);
    }

    std::unique_ptr<ManualMockSessionStart> mock_handler_;
    std::function<crow::response(const crow::request&)> handler_;
};

TEST_F(SessionAuthEndpointTest, ValidAuthorizationFlow) {
    mock_handler_->response_ = {{"token", "valid_token_123"}};
    
    crow::request req;
    req.body = R"({"email":"valid@test.com","password_hash":"correct_hash"})";
    
    crow::response res = handler_(req);
    
    EXPECT_EQ(res.code, 200);
    EXPECT_EQ(nlohmann::json::parse(res.body)["token"], "valid_token_123");
}

TEST_F(SessionAuthEndpointTest, MalformedJsonRequest) {
    crow::request req;
    req.body = "{invalid_json}";
    
    crow::response res = handler_(req);
    
    EXPECT_EQ(res.code, 400);
    EXPECT_EQ(nlohmann::json::parse(res.body)["error"], "Invalid JSON format");
}

TEST_F(SessionAuthEndpointTest, InternalServerErrorHandling) {
    class ExceptionSessionStart : public SessionStart {
    public:
        ExceptionSessionStart() : SessionStart(verifier_) {}
        
        nlohmann::json HandleRequest(const nlohmann::json&) override {
            throw std::logic_error("Critical error");
        }
    
    private:
        DummyUserVerifier verifier_;
    };

    ExceptionSessionStart exception_handler;
    auto error_handler = create_session_auth_handler(exception_handler);
    
    crow::request req;
    req.body = "{}";
    
    crow::response res = error_handler(req);
    
    EXPECT_EQ(res.code, 500);
    EXPECT_EQ(nlohmann::json::parse(res.body)["error"], "Internal server error");
}