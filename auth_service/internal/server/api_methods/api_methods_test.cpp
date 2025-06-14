#include "api_methods.h"

#include <crow.h>
#include <curl/curl.h>
#include <gtest/gtest.h>

#include <future>
#include <thread>

#include "../../../storage/config/config.h"
#include "../../../storage/postgres_connect/connect.h"
#include "../../../storage/redis_config/config_redis.h"
#include "../../../storage/redis_connect/connect_redis.h"
#include "../../auth/user_verify_http/session_hold/session_hold.h"
#include "../../auth/user_verify_http/session_start/session_start.h"

/**
 * @brief Callback-функция для записи данных из CURL-запроса.
 *
 * Эта функция используется c `curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
 * CurlWriteCallback)` для сохранения ответа сервера в строковую переменную.
 *
 * @param contents Указатель на полученные данные.
 * @param size Размер одного элемента данных.
 * @param nmemb Количество элементов данных.
 * @param s Указатель на строку, в которую будут записаны данные.
 * @return Общее количество записанных байт.
 */
static size_t CurlWriteCallback(void* contents, size_t size, size_t nmemb,
                                std::string* s) {
  s->append((char*)contents, size * nmemb);
  return size * nmemb;
}

/**
 * @brief Реализация SessionStart для интеграционных тестов.
 *
 * Инициализирует все необходимые зависимости для работы SessionStart с
 * реальными подключениями к тестовым базам данных.
 */
class RealSessionStart : public SessionStart {
 public:
  /**
   * @brief Конструктор RealSessionStart.
   *
   * Инициализирует внутренние зависимости с использованием тестовых
   * конфигураций PostgreSQL и Redis.
   */
  RealSessionStart()
      : config_(load_config("database_config/test_postgres_config.json")),
        redis_config_(
            load_redis_config("database_config/test_redis_config.json")),
        conn_(connect_to_database(config_)),
        redis_(connect_to_redis(redis_config_)),
        user_verifier_(conn_, redis_),
        SessionStart(user_verifier_) {}

 private:
  Config config_;
  ConfigRedis redis_config_;
  pqxx::connection conn_;
  sw::redis::Redis redis_;
  UserVerifier user_verifier_;
};

/**
 * @brief Реализация SessionHold для интеграционных тестов.
 *
 * Инициализирует все необходимые зависимости для работы SessionHold с реальными
 * подключениями к тестовой базе данных Redis.
 */
class RealSessionHold : public SessionHold {
 public:
  /**
   * @brief Конструктор RealSessionHold.
   *
   * Инициализирует внутренние зависимости с использованием тестовой
   * конфигурации Redis.
   */
  RealSessionHold()
      : redis_config_(
            load_redis_config("database_config/test_redis_config.json")),
        redis_(connect_to_redis(redis_config_)),
        SessionHold(redis_) {}

 private:
  ConfigRedis redis_config_;
  sw::redis::Redis redis_;
};

/**
 * @brief Фикстура для интеграционных тестов API сервера.
 *
 * Настраивает и запускает тестовый HTTP-сервер Crow перед выполнением тестового
 * набора и останавливает его после завершения.
 */
class ApiServerFixture : public ::testing::Test {
 protected:
  /**
   * @brief Настраивает тестовую среду перед выполнением всех тестов.
   *
   * Регистрирует маршруты API, запускает сервер Crow в отдельном потоке и
   * ожидает его готовности.
   */
  static void SetUpTestSuite() {
    register_routes(app, session_start, session_hold);

    server_thread =
        std::thread([]() { app.port(18082).multithreaded().run(); });

    for (int i = 0; i < 10; ++i) {
      if (is_server_alive()) return;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    FAIL() << "API Server did not start in time";
  }

  /**
   * @brief Очищает тестовую среду после выполнения всех тестов.
   *
   * Останавливает сервер Crow и дожидается завершения его потока.
   */
  static void TearDownTestSuite() {
    app.stop();
    if (server_thread.joinable()) server_thread.join();
  }

  /**
   * @brief Проверяет доступность API сервера.
   *
   * Выполняет HEAD-запрос к эндпоинту `/session_start` и проверяет успешность
   * соединения.
   *
   * @return true, если сервер доступен, false в противном случае.
   */
  static bool is_server_alive() {
    CURL* curl = curl_easy_init();
    if (!curl) return false;

    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:18082/session_start");
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 200L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    return (res == CURLE_OK);
  }

  static inline crow::App<crow::CORSHandler> app;
  static inline RealSessionStart session_start;
  static inline RealSessionHold session_hold;
  static inline std::thread server_thread;
};

/**
 * @brief Проверяет доступность маршрута `/session_start`.
 *
 * Тест отправляет POST-запрос на `/session_start` и проверяет,
 * что запрос выполнен успешно и получен непустой ответ.
 */
TEST_F(ApiServerFixture, SessionStartRouteIsAvailable) {
  CURL* curl = curl_easy_init();
  ASSERT_TRUE(curl != nullptr);

  curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:18082/session_start");
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
 * @brief Проверяет доступность маршрута `/session_refresh`.
 *
 * Тест отправляет POST-запрос на `/session_refresh` и проверяет,
 * что запрос выполнен успешно и получен непустой ответ.
 */
TEST_F(ApiServerFixture, SessionRefreshRouteIsAvailable) {
  CURL* curl = curl_easy_init();
  ASSERT_TRUE(curl != nullptr);

  curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:18082/session_refresh");
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
