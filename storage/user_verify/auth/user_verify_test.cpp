#include <gtest/gtest.h>
#include <pqxx/pqxx>
#include "user_verify.h"
#include "../../../uuid_generator/uuid_generator.h"
#include "../../config/config.h"
#include "../../postgres_connect/connect.h"

/**
 * @brief Тестовый класс для UserStorage, использующий реальное соединение с базой данных.
 *
 * Настраивает и очищает тестовые данные в базе данных для каждого теста.
 */
class UserStorageProdTest : public ::testing::Test {
protected:
    pqxx::connection* conn;
    std::string test_user_id;
    std::string test_email;
    UUIDGenerator uuid_gen;

    /**
     * @brief Настраивает тестовую среду перед каждым тестом.
     *
     * Генерирует уникальный ID пользователя и email, устанавливает соединение с тестовой базой данных
     * и вставляет тестового пользователя.
     */
    void SetUp() override {
        test_user_id = uuid_gen.generateUUID();
        test_email = "test_" + test_user_id + "@example.com";

        Config config = load_config("database_config/test_postgres_config.json");
        conn = new pqxx::connection(connect_to_database(config));

        pqxx::work setup_work(*conn);
        setup_work.exec_params(
            "INSERT INTO users (id, username, email, password_hash) "
            "VALUES ($1, $2, $3, $4)",
            test_user_id,
            "test_user",
            test_email,
            "test_hash"
        );
        setup_work.commit();
    }

    /**
     * @brief Очищает тестовую среду после каждого теста.
     *
     * Удаляет тестового пользователя из базы данных и закрывает соединение.
     */
    void TearDown() override {
        pqxx::work teardown_work(*conn);
        teardown_work.exec_params(
            "DELETE FROM users WHERE id = $1",
            test_user_id
        );
        teardown_work.commit();

        delete conn;
    }
};

/**
 * @brief Проверяет, что созданный пользователь существует в базе данных.
 *
 * Тест получает пользователя по email и проверяет, что его ID и email соответствуют ожидаемым.
 */
TEST_F(UserStorageProdTest, CreatedUserExists) {
    UserStorage storage(*conn);
    User user = storage.GetUserByEmail(test_email);
    
    ASSERT_FALSE(user.id.empty());
    EXPECT_EQ(user.id, test_user_id);
    EXPECT_EQ(user.email, test_email);
}

/**
 * @brief Проверяет функциональность верификации пароля.
 *
 * Тест получает пользователя по email и проверяет правильность верификации пароля как для верного, так и для неверного хеша.
 */
TEST_F(UserStorageProdTest, PasswordVerificationWorks) {
    UserStorage storage(*conn);
    User user = storage.GetUserByEmail(test_email);
    
    EXPECT_TRUE(storage.VerifyPassword(user, "test_hash"));
    EXPECT_FALSE(storage.VerifyPassword(user, "wrong_hash"));
}
