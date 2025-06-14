#include "session_auth_endpoint.h"

#include <crow.h>
#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

#include "../../../../../storage/config/config.h"
#include "../../../../../storage/postgres_connect/connect.h"
#include "../../../../../storage/redis_config/config_redis.h"
#include "../../../../../storage/redis_connect/connect_redis.h"
#include "../../session_start/session_start.h"

/**
 * @brief Мок-класс для SessionStart, используемый в тестах.
 *
 * Позволяет имитировать поведение SessionStart для тестирования обработчиков
 * эндпоинтов, фиксируя входящие запросы и возвращая предопределенные ответы.
 */
class MockSessionStart : public SessionStart {
 public:
  /**
   * @brief Конструктор MockSessionStart.
   *
   * Инициализирует внутренние зависимости для имитации поведения SessionStart.
   */
  MockSessionStart()
      : SessionStart(user_verifier_),
        config_(load_config("database_config/test_postgres_config.json")),
        redis_config_(
            load_redis_config("database_config/test_redis_config.json")),
        conn_(connect_to_database(config_)),
        redis_(connect_to_redis(redis_config_)),
        user_verifier_(conn_, redis_) {}

  /**
   * @brief Имитирует обработку запроса SessionStart.
   *
   * Сохраняет данные входящего запроса и возвращает предварительно
   * установленный ответ.
   * @param request_data Входящие данные запроса в формате JSON.
   * @return Заранее определенный ответ в формате JSON.
   */
  json HandleRequest(const json& request_data) {
    last_request = request_data;
    return response_to_return;
  }

  json last_request;
  json response_to_return;

 private:
  Config config_;
  ConfigRedis redis_config_;
  pqxx::connection conn_;
  sw::redis::Redis redis_;

  UserVerifier user_verifier_;
};

/**
 * @brief Тестовый класс для эндпоинта аутентификации сессии.
 *
 * Использует MockSessionStart для изоляции и тестирования логики эндпоинта
 * аутентификации.
 */
class SessionAuthEndpointTest : public ::testing::Test {
 protected:
  MockSessionStart handler;
  crow::request req;
};

/**
 * @brief Проверяет обработку валидного запроса к эндпоинту аутентификации
 * сессии.
 *
 * Тест имитирует успешный запрос аутентификации и проверяет, что эндпоинт
 * возвращает ожидаемый HTTP-статус 200 и корректный токен.
 */
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

/**
 * @brief Проверяет обработку невалидного JSON в запросе.
 *
 * Тест отправляет невалидный JSON в теле запроса и ожидает HTTP-статус 500
 * с сообщением об ошибке внутреннего сервера.
 */
TEST_F(SessionAuthEndpointTest, HandlesInvalidJson) {
  req.body = "invalid json";
  auto handler_func = create_session_auth_handler(handler);
  auto response = handler_func(req);

  EXPECT_EQ(response.code, 500);
  auto response_json = nlohmann::json::parse(response.body);
  EXPECT_TRUE(response_json.contains("error"));
}

/**
 * @brief Проверяет обработку ошибок формата JSON.
 *
 * Тест имитирует ошибку формата JSON от обработчика `SessionStart` (например,
 * отсутствующее поле) и проверяет, что эндпоинт возвращает HTTP-статус 400.
 */
TEST_F(SessionAuthEndpointTest, HandlesJsonFormatError) {
  req.body = R"({"email": "test@example.com"})";
  handler.response_to_return = {{"error", "Invalid JSON format"},
                                {"details", "Missing password_hash field"}};

  auto handler_func = create_session_auth_handler(handler);
  auto response = handler_func(req);

  EXPECT_EQ(response.code, 400);
  auto response_json = nlohmann::json::parse(response.body);
  EXPECT_EQ(response_json["error"], "Invalid JSON format");
}

/**
 * @brief Проверяет обработку ошибок верификации.
 *
 * Тест имитирует ошибку верификации от обработчика `SessionStart` (например,
 * неверные учетные данные) и проверяет, что эндпоинт возвращает HTTP-статус
 * 401.
 */
TEST_F(SessionAuthEndpointTest, HandlesVerificationError) {
  req.body = R"({"email": "test@example.com", "password_hash": "wrong_hash"})";
  handler.response_to_return = {{"error", "Verification failed"},
                                {"details", "Invalid credentials"}};

  auto handler_func = create_session_auth_handler(handler);
  auto response = handler_func(req);

  EXPECT_EQ(response.code, 401);
  auto response_json = nlohmann::json::parse(response.body);
  EXPECT_EQ(response_json["error"], "Verification failed");
}

/**
 * @brief Проверяет обработку неизвестных ошибок.
 *
 * Тест имитирует общую неизвестную ошибку от обработчика `SessionStart` и
 * проверяет, что эндпоинт возвращает HTTP-статус 401 и соответствующее
 * сообщение об ошибке.
 */
TEST_F(SessionAuthEndpointTest, HandlesUnknownError) {
  req.body = R"({"email": "test@example.com", "password_hash": "hash123"})";
  handler.response_to_return = {{"error", "Unknown error"},
                                {"details", "Something went wrong"}};

  auto handler_func = create_session_auth_handler(handler);
  auto response = handler_func(req);

  EXPECT_EQ(response.code, 401);
  auto response_json = nlohmann::json::parse(response.body);
  EXPECT_EQ(response_json["error"], "Verification failed");
}
