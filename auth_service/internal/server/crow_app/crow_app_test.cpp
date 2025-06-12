#include <gtest/gtest.h>
#include <crow.h>
#include <thread>
#include <curl/curl.h>
#include "crow_app.h"
#include "../../auth/user_verify_http/session_start/session_start.h"
#include "../../auth/user_verify_http/session_hold/session_hold.h"
#include "../../../storage/config/config.h"
#include "../../../storage/redis_config/config_redis.h"
#include "../../../storage/postgres_connect/connect.h"
#include "../../../storage/redis_connect/connect_redis.h"

static size_t CurlWriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    s->append((char*)contents, size * nmemb);
    return size * nmemb;
}

class RealSessionStart : public SessionStart {
public:
    RealSessionStart()
        : config_(load_config("database_config/test_postgres_config.json")),
          redis_config_(load_redis_config("database_config/test_redis_config.json")),
          conn_(connect_to_database(config_)),
          redis_(connect_to_redis(redis_config_)),
          user_verifier_(conn_, redis_),
          SessionStart(user_verifier_)
    {}

private:
    Config config_;
    ConfigRedis redis_config_;
    pqxx::connection conn_;
    sw::redis::Redis redis_;
    UserVerifier user_verifier_;
};

class RealSessionHold : public SessionHold {
public:
    RealSessionHold()
        : redis_config_(load_redis_config("database_config/test_redis_config.json")),
          redis_(connect_to_redis(redis_config_)),
          SessionHold(redis_)
    {}

private:
    ConfigRedis redis_config_;
    sw::redis::Redis redis_;
};

class CrowAppServerFixture : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        app = &create_crow_app(session_start, session_hold);

        server_thread = std::thread([](){
            app->port(18081).multithreaded().run();
        });

        for (int i = 0; i < 10; ++i) {
            if (is_server_alive()) return;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        FAIL() << "CrowApp: Server did not start in time";
    }

    static void TearDownTestSuite() {
        app->stop();
        if (server_thread.joinable())
            server_thread.join();
    }

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
    static inline RealSessionStart session_start;
    static inline RealSessionHold session_hold;
    static inline std::thread server_thread;
};

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
