#include "user_verify.h"

#include <gtest/gtest.h>
#include <sw/redis++/redis++.h>

#include <pqxx/pqxx>

#include "../../../../storage/config/config.h"
#include "../../../../storage/postgres_connect/connect.h"
#include "../../../../storage/redis_config/config_redis.h"
#include "../../../../storage/redis_connect/connect_redis.h"
#include "../../../../uuid_generator/uuid_generator.h"
#include "../../../models/user.h"

const std::string VALID_PASSWORD_HASH =
    "5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8";

/**
 * @brief Тестовый класс для UserVerifier.
 *
 * Настраивает и очищает тестовую среду, включая подключение к PostgreSQL и
 * Redis, а также вставку и удаление тестовых пользователей.
 */
class UserVerifierTest : public ::testing::Test {
 protected:
  /**
   * @brief Настраивает тестовую среду перед каждым тестом.
   *
   * Устанавливает соединения с тестовыми базами данных PostgreSQL и Redis,
   * генерирует тестового пользователя и вставляет его в базу данных.
   */
  void SetUp() override {
    try {
      Config postgres_config =
          load_config("database_config/test_postgres_config.json");

      conn = std::make_unique<pqxx::connection>(
          connect_to_database(postgres_config));

      ConfigRedis redis_config =
          load_redis_config("database_config/test_redis_config.json");

      redis =
          std::make_unique<sw::redis::Redis>(connect_to_redis(redis_config));

      uuidGenerator = std::make_unique<UUIDGenerator>();

      testUserId = uuidGenerator->generateUUID();
      testEmail = "test_" + testUserId + "@example.com";

      insertTestUser();
    } catch (const std::exception& e) {
      FAIL() << "Setup failed: " << e.what();
    }
  }

  /**
   * @brief Очищает тестовую среду после каждого теста.
   *
   * Удаляет тестового пользователя и связанные с ним данные из базы данных.
   */
  void TearDown() override {
    pqxx::work txn(*conn);
    try {
      txn.exec_params(
          "DELETE FROM transfers "
          "WHERE from_account IN (SELECT id FROM accounts WHERE user_id = $1) "
          "OR to_account IN (SELECT id FROM accounts WHERE user_id = $1)",
          testUserId);

      txn.exec_params("DELETE FROM accounts WHERE user_id = $1", testUserId);

      txn.exec_params("DELETE FROM users WHERE id = $1", testUserId);

      txn.commit();
    } catch (const std::exception& e) {
      txn.abort();
      FAIL() << "Cleanup failed: " << e.what();
    }
  }

  /**
   * @brief Вставляет тестового пользователя в базу данных.
   *
   * Вставляет запись о тестовом пользователе с предопределенными данными и
   * хешем пароля в таблицу `users`.
   */
  void insertTestUser() {
    pqxx::work txn(*conn);
    try {
      txn.exec_params(
          "INSERT INTO users (id, username, email, password_hash) "
          "VALUES ($1, $2, $3, $4)",
          testUserId, "testuser", testEmail, VALID_PASSWORD_HASH);
      txn.commit();
    } catch (const std::exception& e) {
      txn.abort();
      FAIL() << "Failed to insert test user: " << e.what();
    }
  }

  std::unique_ptr<pqxx::connection> conn;
  std::unique_ptr<UUIDGenerator> uuidGenerator;
  std::unique_ptr<sw::redis::Redis> redis;
  std::string testUserId;
  std::string testEmail;
};

/**
 * @brief Проверяет, что при неверном email выбрасывается исключение.
 *
 * Тест пытается сгенерировать токен с несуществующим email и ожидает
 * `std::runtime_error`.
 */
TEST_F(UserVerifierTest, ThrowsExceptionForWrongEmail) {
  UserVerifier verifier(*conn, *redis);

  EXPECT_THROW(
      {
        verifier.GenerateToken("wrong_" + testUserId + "@example.com",
                               VALID_PASSWORD_HASH);
      },
      std::runtime_error);
}

/**
 * @brief Проверяет, что при неверном пароле выбрасывается исключение.
 *
 * Тест пытается сгенерировать токен с неверным хешем пароля для существующего
 * email и ожидает `std::runtime_error`.
 */
TEST_F(UserVerifierTest, ThrowsExceptionForWrongPassword) {
  UserVerifier verifier(*conn, *redis);

  EXPECT_THROW(
      { verifier.GenerateToken(testEmail, "wrong_hash"); }, std::runtime_error);
}
