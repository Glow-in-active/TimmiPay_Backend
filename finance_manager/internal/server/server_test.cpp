#include <gtest/gtest.h>
#include "server.h"
#include "../../../../storage/config/config.h"
#include "../../../../storage/redis_config/config_redis.h"
#include "../../../../storage/postgres_connect/connect.h"
#include "../../../../storage/redis_connect/connect_redis.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <thread>
#include <chrono>
#include "../../../../uuid_generator/uuid_generator.h"

/**
 * @brief Тестовый класс для интеграционного тестирования FinanceServer.
 *
 * Настраивает и очищает тестовую базу данных и сервер Crow для каждого тестового набора.
 */
class ServerTest : public ::testing::Test {
protected:
    // Static members for server and connections, initialized once for all tests
    static inline std::unique_ptr<pqxx::connection> postgres_conn;
    static inline std::unique_ptr<sw::redis::Redis> redis_conn;
    static inline std::unique_ptr<FinanceServer> server;
    static inline std::thread server_thread;

    std::string test_user1_id;
    std::string test_user2_id;
    std::string test_usd_id;
    std::string test_eur_id;
    std::string test_session_token;
    UUIDGenerator uuid_gen;

    /**
     * @brief Настраивает тестовый набор перед выполнением всех тестов.
     *
     * Инициализирует статические соединения с PostgreSQL и Redis, создает и запускает
     * FinanceServer в отдельном потоке, а затем ожидает его готовности.
     */
    static void SetUpTestSuite() {
        Config postgres_config = load_config("database_config/test_postgres_config.json");
        ConfigRedis redis_config = load_redis_config("database_config/test_redis_config.json");

        postgres_conn = std::make_unique<pqxx::connection>(connect_to_database(postgres_config));
        redis_conn = std::make_unique<sw::redis::Redis>(connect_to_redis(redis_config));

        server = std::make_unique<FinanceServer>(*postgres_conn, *redis_conn);
        
        server_thread = std::thread([]() {
            server->run(8080);
        });
        
        for (int i = 0; i < 100; ++i) { 
            if (is_server_alive()) return;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        FAIL() << "CrowApp: Server did not start in time";
    }

    /**
     * @brief Очищает ресурсы тестового набора после выполнения всех тестов.
     *
     * Останавливает FinanceServer и ожидает завершения его потока. Соединения с базами данных
     * будут закрыты автоматически при уничтожении `std::unique_ptr`.
     */
    static void TearDownTestSuite() {
        // Stop server
        if (server) {
            server->stop_server();
        }
        if (server_thread.joinable()) {
            server_thread.join();
        }

    }

    void SetUp() override {
        setupTestData();
    }

    void TearDown() override {
        cleanupTestData();
    }

    /**
     * @brief Настраивает тестовые данные для каждого теста.
     *
     * Вставляет тестовых пользователей, валюты и счета в базу данных PostgreSQL,
     * а также создает тестовый токен сессии в Redis.
     */
    void setupTestData() {
        pqxx::work txn(*postgres_conn);
        
        test_user1_id = uuid_gen.generateUUID();
        test_user2_id = uuid_gen.generateUUID();
        txn.exec_params("INSERT INTO users (id, username, email, password_hash) VALUES ($1, 'test_user1', 'test1@example.com', 'password_hash_1')", test_user1_id);
        txn.exec_params("INSERT INTO users (id, username, email, password_hash) VALUES ($1, 'test_user2', 'test2@example.com', 'password_hash_2')", test_user2_id);
        
        test_usd_id = uuid_gen.generateUUID();
        test_eur_id = uuid_gen.generateUUID();
        txn.exec_params("INSERT INTO currencies (id, code, name) VALUES ($1, 'USD', 'United States Dollar')", test_usd_id);
        txn.exec_params("INSERT INTO currencies (id, code, name) VALUES ($1, 'EUR', 'Euro')", test_eur_id);
        
        txn.exec_params("INSERT INTO accounts (id, user_id, currency_id, balance) VALUES ($1, $2, $3, 1000.0)", uuid_gen.generateUUID(), test_user1_id, test_usd_id);
        txn.exec_params("INSERT INTO accounts (id, user_id, currency_id, balance) VALUES ($1, $2, $3, 500.0)", uuid_gen.generateUUID(), test_user2_id, test_usd_id);
        
        txn.commit();

        test_session_token = uuid_gen.generateUUID();
        redis_conn->hset(test_session_token, "id", test_user1_id);
        redis_conn->expire(test_session_token, 60);
    }

    /**
     * @brief Очищает тестовые данные после каждого теста.
     *
     * Удаляет тестовых пользователей, счета, валюты из PostgreSQL и тестовый токен сессии из Redis.
     */
    void cleanupTestData() {
        pqxx::work txn(*postgres_conn);
        txn.exec("DELETE FROM transfers");
        txn.exec("DELETE FROM accounts");
        txn.exec("DELETE FROM currencies");
        txn.exec("DELETE FROM users");
        txn.commit();

        redis_conn->hdel(test_session_token, "id");
    }

    /**
     * @brief Callback-функция для записи данных, полученных от cURL.
     *
     * @param contents Указатель на полученные данные.
     * @param size Размер одного элемента данных.
     * @param nmemb Количество элементов данных.
     * @param userp Указатель на строку, куда будут добавлены данные.
     * @return Фактическое количество записанных байт.
     */
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
        userp->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    /**
     * @brief Выполняет HTTP-запрос к серверу.
     *
     * @param endpoint Конечная точка API (например, "/api/v1/balance").
     * @param method HTTP-метод (например, "POST").
     * @param data Тело запроса в виде строки JSON.
     * @return Строка, содержащая ответ сервера.
     * @throws std::runtime_error Если запрос cURL завершается с ошибкой.
     */
    std::string makeRequest(const std::string& endpoint, const std::string& method, const std::string& data) {
        CURL* curl = curl_easy_init();
        std::string response_string;

        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, ("http://localhost:8080" + endpoint).c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
            
            if (!data.empty()) {
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
            }

            CURLcode res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);

            if (res != CURLE_OK) {
                throw std::runtime_error("Curl request failed");
            }
        }

        return response_string;
    }

    /**
     * @brief Проверяет, запущен ли сервер.
     *
     * Отправляет небольшой запрос к серверу, чтобы убедиться, что он отвечает.
     * @return true, если сервер отвечает, false в противном случае.
     */
    static bool is_server_alive() {
        CURL* curl = curl_easy_init();
        if (!curl) return false;

        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8080/api/v1/balance");
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 200L);

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        return res == CURLE_OK;
    }
};

/**
 * @brief Проверяет успешное получение баланса пользователя.
 *
 * Тест отправляет запрос на получение баланса и проверяет, что ответ содержит
 * корректные данные о валюте и балансе пользователя.
 */
TEST_F(ServerTest, GetBalanceSuccess) {
    nlohmann::json request_data = {
        {"session_token", test_session_token}
    };

    std::string response = makeRequest("/api/v1/balance", "POST", request_data.dump());
    auto response_json = nlohmann::json::parse(response);

    ASSERT_TRUE(response_json.is_array());
    ASSERT_EQ(response_json.size(), 1);
    EXPECT_EQ(response_json[0]["currency"], "USD");
    EXPECT_DOUBLE_EQ(response_json[0]["balance"], 1000.0);
}

/**
 * @brief Проверяет обработку недействительного токена сессии при получении баланса.
 *
 * Тест отправляет запрос на получение баланса с неверным токеном сессии
 * и проверяет, что сервер возвращает ошибку 401.
 */
TEST_F(ServerTest, GetBalanceInvalidSession) {
    nlohmann::json request_data = {
        {"session_token", "invalid_token"}
    };

    std::string response = makeRequest("/api/v1/balance", "POST", request_data.dump());
    EXPECT_EQ(response, "Invalid session token");
}

/**
 * @brief Проверяет успешный перевод денег через API.
 *
 * Тест отправляет запрос на перевод денег и проверяет, что ответ содержит
 * непустой `transfer_id`, подтверждающий успешное создание транзакции.
 */
TEST_F(ServerTest, TransferMoneySuccess) {
    nlohmann::json request_data = {
        {"session_token", test_session_token},
        {"to_username", "test_user2"},
        {"amount", 100.0},
        {"currency", "USD"}
    };

    std::string response = makeRequest("/api/v1/transfer", "POST", request_data.dump());
    auto response_json = nlohmann::json::parse(response);

    EXPECT_TRUE(response_json.contains("transfer_id"));
    EXPECT_FALSE(response_json["transfer_id"].empty());
}

/**
 * @brief Проверяет обработку недостаточных средств при переводе денег.
 *
 * Тест отправляет запрос на перевод суммы, превышающей доступный баланс пользователя,
 * и проверяет, что сервер возвращает соответствующее сообщение об ошибке.
 */
TEST_F(ServerTest, TransferMoneyInsufficientFunds) {
    nlohmann::json request_data = {
        {"session_token", test_session_token},
        {"to_username", "test_user2"},
        {"amount", 2000.0},
        {"currency", "USD"}
    };

    std::string response = makeRequest("/api/v1/transfer", "POST", request_data.dump());
    EXPECT_EQ(response, "Insufficient funds.");
}

/**
 * @brief Проверяет получение истории транзакций пользователя.
 *
 * Тест сначала выполняет перевод, а затем отправляет запрос на получение истории транзакций.
 * Проверяет, что ответ содержит корректные данные о транзакции, включая сумму и статус.
 */
TEST_F(ServerTest, GetTransactionHistory) {
    // Make a transfer first
    nlohmann::json transfer_data = {
        {"session_token", test_session_token},
        {"to_username", "test_user2"},
        {"amount", 100.0},
        {"currency", "USD"}
    };
    makeRequest("/api/v1/transfer", "POST", transfer_data.dump());

    // Get history
    nlohmann::json history_data = {
        {"session_token", test_session_token},
        {"page", 1},
        {"limit", 10}
    };

    std::string response = makeRequest("/api/v1/history", "POST", history_data.dump());
    auto response_json = nlohmann::json::parse(response);

    ASSERT_TRUE(response_json.is_array());
    ASSERT_EQ(response_json.size(), 1);
    EXPECT_DOUBLE_EQ(response_json[0]["amount"], 100.0);
    EXPECT_EQ(response_json[0]["status"], "completed");
} 