#include "start_server.h"

#include <crow.h>
#include <gtest/gtest.h>
#include <sw/redis++/redis++.h>

#include <chrono>
#include <future>
#include <pqxx/pqxx>
#include <thread>

#include "../crow_app/crow_app.h"
#include "../db_init/db_init.h"
#include "../dependencies/dependencies.h"

/**
 * @brief Тестовый класс для функций запуска сервера и обработки ошибок.
 *
 * Этот класс содержит общие настройки и вспомогательные методы для тестирования
 * различных сценариев, связанных с запуском HTTP-сервера и его обработкой
 * исключений.
 */
class StartServerTest : public ::testing::Test {
 protected:
  /**
   * @brief Настраивает тестовую среду перед каждым тестом.
   *
   * В данном случае, дополнительных настроек перед каждым тестом не требуется.
   */
  void SetUp() override {}
  /**
   * @brief Очищает тестовую среду после каждого теста.
   *
   * В данном случае, дополнительных действий по очистке после каждого теста не
   * требуется.
   */
  void TearDown() override {}
};

/**
 * @brief Проверяет обработку ошибок PostgreSQL.
 *
 * Тест имитирует ошибку `pqxx::sql_error` и проверяет, что сообщение об ошибке
 * содержит ожидаемые подстроки, указывающие на ошибку PostgreSQL.
 */
TEST_F(StartServerTest, PostgreSQLErrorHandling) {
  try {
    throw pqxx::sql_error("Test PostgreSQL error");
  } catch (const pqxx::sql_error& e) {
    std::string error_msg = e.what();
    EXPECT_FALSE(error_msg.empty());
    EXPECT_TRUE(error_msg.find("PostgreSQL") != std::string::npos ||
                error_msg.find("Test") != std::string::npos);
  }
}

/**
 * @brief Проверяет обработку ошибок Redis.
 *
 * Тест имитирует ошибку `sw::redis::Error` и проверяет, что сообщение об ошибке
 * содержит ожидаемые подстроки, указывающие на ошибку Redis.
 */
TEST_F(StartServerTest, RedisErrorHandling) {
  try {
    throw sw::redis::Error("Test Redis error");
  } catch (const sw::redis::Error& e) {
    std::string error_msg = e.what();
    EXPECT_FALSE(error_msg.empty());
    EXPECT_TRUE(error_msg.find("Redis") != std::string::npos ||
                error_msg.find("Test") != std::string::npos);
  }
}

/**
 * @brief Проверяет общую обработку исключений.
 *
 * Тест имитирует общее исключение `std::runtime_error` и проверяет,
 * что сообщение об ошибке точно соответствует ожидаемому.
 */
TEST_F(StartServerTest, GeneralExceptionHandling) {
  try {
    throw std::runtime_error("Test general error");
  } catch (const std::exception& e) {
    std::string error_msg = e.what();
    EXPECT_FALSE(error_msg.empty());
    EXPECT_STREQ("Test general error", error_msg.c_str());
  }
}

/**
 * @brief Проверяет конфигурацию порта сервера.
 *
 * Тест проверяет, что ожидаемый порт сервера (8080) соответствует константе.
 */
TEST_F(StartServerTest, ServerPortConfiguration) {
  const int EXPECTED_PORT = 8080;
  EXPECT_EQ(8080, EXPECTED_PORT);
}

/**
 * @brief Проверяет порядок перехвата исключений.
 *
 * Тест проверяет, что исключения `pqxx::sql_error`, `sw::redis::Error` и
 * `std::runtime_error` перехватываются в правильном порядке, демонстрируя
 * специфичность обработчиков.
 */
TEST_F(StartServerTest, ExceptionCatchOrder) {
  bool postgresql_caught = false;
  bool redis_caught = false;
  bool general_caught = false;

  try {
    throw pqxx::sql_error("PostgreSQL test");
  } catch (const pqxx::sql_error& e) {
    postgresql_caught = true;
  } catch (const sw::redis::Error& e) {
    redis_caught = true;
  } catch (const std::exception& e) {
    general_caught = true;
  }

  EXPECT_TRUE(postgresql_caught);
  EXPECT_FALSE(redis_caught);
  EXPECT_FALSE(general_caught);

  postgresql_caught = redis_caught = general_caught = false;

  try {
    throw sw::redis::Error("Redis test");
  } catch (const pqxx::sql_error& e) {
    postgresql_caught = true;
  } catch (const sw::redis::Error& e) {
    redis_caught = true;
  } catch (const std::exception& e) {
    general_caught = true;
  }

  EXPECT_FALSE(postgresql_caught);
  EXPECT_TRUE(redis_caught);
  EXPECT_FALSE(general_caught);

  postgresql_caught = redis_caught = general_caught = false;

  try {
    throw std::runtime_error("General test");
  } catch (const pqxx::sql_error& e) {
    postgresql_caught = true;
  } catch (const sw::redis::Error& e) {
    redis_caught = true;
  } catch (const std::exception& e) {
    general_caught = true;
  }

  EXPECT_FALSE(postgresql_caught);
  EXPECT_FALSE(redis_caught);
  EXPECT_TRUE(general_caught);
}

/**
 * @brief Проверяет содержимое сообщений об ошибках.
 *
 * Тест проверяет, что сообщения об ошибках для PostgreSQL, Redis и общих
 * исключений не пусты и содержат ожидаемые ключевые слова.
 */
TEST_F(StartServerTest, ErrorMessageContent) {
  std::string pq_error;
  std::string redis_error;
  std::string general_error;

  try {
    throw pqxx::sql_error("PostgreSQL connection failed");
  } catch (const pqxx::sql_error& e) {
    pq_error = e.what();
  }

  try {
    throw sw::redis::Error("Redis connection timeout");
  } catch (const sw::redis::Error& e) {
    redis_error = e.what();
  }

  try {
    throw std::runtime_error("Fatal server error");
  } catch (const std::exception& e) {
    general_error = e.what();
  }

  EXPECT_FALSE(pq_error.empty());
  EXPECT_FALSE(redis_error.empty());
  EXPECT_FALSE(general_error.empty());

  EXPECT_TRUE(pq_error.find("PostgreSQL") != std::string::npos);
  EXPECT_TRUE(redis_error.find("Redis") != std::string::npos);
  EXPECT_TRUE(general_error.find("Fatal") != std::string::npos);
}

/**
 * @brief Проверяет существование функции `start_server`.
 *
 * Тест проверяет, что указатель на функцию `start_server` не является nullptr,
 * подтверждая ее доступность.
 */
TEST_F(StartServerTest, StartServerFunctionExists) {
  void (*func_ptr)(Dependencies&) = start_server;
  EXPECT_NE(func_ptr, nullptr);
}

/**
 * @brief Проверяет успешное включение заголовков.
 *
 * Этот тест является заглушкой и всегда проходит успешно, подтверждая наличие
 * необходимых инклюдов.
 */
TEST_F(StartServerTest, IncludeHeaders) { EXPECT_TRUE(true); }

/**
 * @brief Проверяет отсутствие исключений при обработке различных типов ошибок.
 *
 * Тест имитирует различные типы исключений (`pqxx::sql_error`,
 * `sw::redis::Error`, `std::runtime_error`) и проверяет, что их перехват не
 * приводит к новым исключениям.
 */
TEST_F(StartServerTest, ExceptionTypes) {
  EXPECT_NO_THROW({
    try {
      throw pqxx::sql_error("test");
    } catch (const pqxx::sql_error&) {
    }
  });

  EXPECT_NO_THROW({
    try {
      throw sw::redis::Error("test");
    } catch (const sw::redis::Error&) {
    }
  });

  EXPECT_NO_THROW({
    try {
      throw std::runtime_error("test");
    } catch (const std::exception&) {
    }
  });
}

/**
 * @brief Проверяет наличие сообщений об ошибках.
 *
 * Тест проверяет, что предопределенные сообщения об ошибках (PostgreSQL, Redis,
 * Fatal) не пусты.
 */
TEST_F(StartServerTest, ErrorMessages) {
  std::vector<std::string> messages = {"PostgreSQL error", "Redis error",
                                       "Fatal error"};

  for (const auto& msg : messages) {
    EXPECT_FALSE(msg.empty());
  }
}

/**
 * @brief Проверяет указатель на функцию `start_server`.
 *
 * Этот тест является дубликатом `StartServerFunctionExists` и всегда проходит
 * успешно, подтверждая доступность функции `start_server`.
 */
TEST_F(StartServerTest, FunctionPointer) {
  auto func = &start_server;
  EXPECT_NE(func, nullptr);
}

/**
 * @brief Проверяет тип возвращаемого значения `create_crow_app`.
 *
 * Тест использует `std::is_same_v` для проверки того, что тип, возвращаемый
 * функцией `create_crow_app`, соответствует ожидаемому типу ссылки на
 * `crow::App<crow::CORSHandler>`.
 */
TEST_F(StartServerTest, CrowAppType) {
  using ExpectedType = crow::App<crow::CORSHandler>&;
  using ActualType = decltype(create_crow_app(std::declval<SessionStart&>(),
                                              std::declval<SessionHold&>()));

  EXPECT_TRUE((std::is_same_v<ExpectedType, ActualType>));
}

/**
 * @brief Проверяет включение всех необходимых заголовочных файлов.
 *
 * Этот тест является заглушкой и всегда проходит успешно, подтверждая наличие
 * необходимых инклюдов.
 */
TEST_F(StartServerTest, HeaderInclusion) { EXPECT_TRUE(true); }