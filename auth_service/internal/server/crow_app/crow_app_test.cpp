#include "crow_app.h"

#include <crow.h>
#include <curl/curl.h>
#include <gtest/gtest.h>

#include <nlohmann/json.hpp>
#include <memory>
#include <thread>

#include "../../../storage/config/config.h"
#include "../../../storage/postgres_connect/connect.h"
#include "../../../storage/redis_config/config_redis.h"
#include "../../../storage/redis_connect/connect_redis.h"
#include "../db_init/db_init.h"
#include "../dependencies/dependencies.h"

/**
 * @brief Callback-функция для записи данных, полученных от cURL.
 *
 * @param contents Указатель на полученные данные.
 * @param size Размер одного элемента данных.
 * @param nmemb Количество элементов данных.
 * @param s Указатель на строку, куда будут добавлены данные.
 * @return Фактическое количество записанных байт.
 */
static size_t CurlWriteCallback(void* contents, size_t size, size_t nmemb,
                                std::string* s) {
  s->append((char*)contents, size * nmemb);
  return size * nmemb;
}

/**
 * @brief Фикстура для интеграционных тестов сервера CrowApp.
 *
 * Настраивает и запускает тестовый HTTP-сервер Crow перед выполнением тестового
 * набора и останавливает его после завершения.
 */
class CrowAppServerFixture : public ::testing::Test {
 protected:
  /**
   * @brief Настраивает тестовый набор перед выполнением всех тестов.
   *
   * Инициализирует приложение Crow, запускает его в отдельном потоке и ожидает
   * готовности сервера.
   */
  static void SetUpTestSuite() {
    db_connections = std::make_unique<DBConnections>(initialize_databases());
    deps = std::make_unique<Dependencies>(initialize_dependencies(*db_connections));
    app = &create_crow_app(*deps);

    server_thread =
        std::thread([]() { app->port(18081).multithreaded().run(); });

    for (int i = 0; i < 10; ++i) {
      if (is_server_alive()) return;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    FAIL() << "CrowApp: Server did not start in time";
  }

  /**
   * @brief Очищает ресурсы после выполнения всех тестов.
   *
   * Останавливает приложение Crow и завершает поток сервера.
   */
  static void TearDownTestSuite() {
    app->stop();
    if (server_thread.joinable()) server_thread.join();
  }

  /**
   * @brief Проверяет доступность сервера.
   *
   * Отправляет HTTP-запрос к серверу и проверяет успешность соединения.
   *
   * @return true, если сервер доступен, false в противном случае.
   */
  static bool is_server_alive() {
    CURL* curl = curl_easy_init();
    if (!curl) return false;

    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:18081/session_start");
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 200L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    return res == CURLE_OK;
  }

  static inline crow::App<crow::CORSHandler>* app;
  static inline std::unique_ptr<DBConnections> db_connections;
  static inline std::unique_ptr<Dependencies> deps;
  static inline std::thread server_thread;
};

/**
 * @brief Проверяет доступность маршрута /session_start.
 *
 * Тест отправляет POST-запрос на /session_start и проверяет, что ответ получен
 * успешно и не пуст.
 */
TEST_F(CrowAppServerFixture, SessionStartRouteIsAvailable) {
  CURL* curl = curl_easy_init();
  ASSERT_TRUE(curl != nullptr);

  curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:18081/session_start");
  curl_easy_setopt(curl, CURLOPT_POST, 1L);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "{}");
  curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 1000L);

  std::string response;
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

  CURLcode res = curl_easy_perform(curl);
  curl_easy_cleanup(curl);

  EXPECT_EQ(res, CURLE_OK);
  EXPECT_FALSE(response.empty());
}

/**
 * @brief Проверяет доступность маршрута /session_refresh.
 *
 * Тест отправляет POST-запрос на /session_refresh и проверяет, что ответ
 * получен успешно и не пуст.
 */
TEST_F(CrowAppServerFixture, SessionRefreshRouteIsAvailable) {
  CURL* curl = curl_easy_init();
  ASSERT_TRUE(curl != nullptr);

  curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:18081/session_refresh");
  curl_easy_setopt(curl, CURLOPT_POST, 1L);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "{}");
  curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 1000L);

  std::string response;
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

  CURLcode res = curl_easy_perform(curl);
  curl_easy_cleanup(curl);

  EXPECT_EQ(res, CURLE_OK);
  EXPECT_FALSE(response.empty());
}

/**
 * @brief Проверяет доступность маршрута /register и успешную регистрацию.
 *
 * Тест отправляет POST-запрос на /register с валидными данными и проверяет,
 * что ответ получен успешно.
 */
TEST_F(CrowAppServerFixture, RegistrationRouteIsAvailableAndWorks) {
  CURL* curl = curl_easy_init();
  ASSERT_TRUE(curl != nullptr);

  // Тестовые данные для регистрации
  std::string username = "testuser_register_123";
  std::string email = "register_test@example.com";
  std::string password_hash = "hash_password_123";

  nlohmann::json request_body;
  request_body["username"] = username;
  request_body["email"] = email;
  request_body["password_hash"] = password_hash;

  std::string request_data = request_body.dump();

  curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:18081/register");
  curl_easy_setopt(curl, CURLOPT_POST, 1L);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_data.c_str());
  curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 1000L);

  std::string response_string;
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

  CURLcode res = curl_easy_perform(curl);
  curl_easy_cleanup(curl);

  EXPECT_EQ(res, CURLE_OK);
  EXPECT_FALSE(response_string.empty());

  // Проверяем, что регистрация прошла успешно
  nlohmann::json response_json = nlohmann::json::parse(response_string);
  EXPECT_EQ(response_json["message"], "User registered successfully");

  // Повторная регистрация с теми же данными должна вернуть 409 Conflict
  curl = curl_easy_init();
  ASSERT_TRUE(curl != nullptr);

  curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:18081/register");
  curl_easy_setopt(curl, CURLOPT_POST, 1L);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_data.c_str());
  curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 1000L);

  response_string.clear(); // Очищаем строку для нового ответа
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

  res = curl_easy_perform(curl);
  curl_easy_cleanup(curl);

  EXPECT_EQ(res, CURLE_OK);
  EXPECT_FALSE(response_string.empty());

  // Проверяем, что получена ошибка 409 Conflict
  response_json = nlohmann::json::parse(response_string);
  EXPECT_EQ(response_json["error"], "Пользователь с такими данными уже существует");
}
