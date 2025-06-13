#include "token_generator.h"
#include "../../../../storage/user_verify/redis_set/redis_set_token.h"
#include "../../../../storage/user_verify/auth/user_verify.h"

/**
 * @brief Генерирует новый токен для пользователя и сохраняет его в Redis.
 *
 * Использует UUIDGenerator для создания уникального токена и сохраняет его
 * вместе с ID пользователя в Redis с помощью `set_token`.
 *
 * @param user Объект User, для которого генерируется токен.
 * @return Сгенерированный строковый токен.
 */
std::string TokenGenerator::GenerateToken(const User& user) {
    const std::string token = uuid_gen_.generateUUID();

    set_token(redis_, token, user.id);
    
    return token;
}

/**
 * @brief Конструктор класса TokenGenerator.
 *
 * Инициализирует TokenGenerator с необходимыми зависимостями.
 *
 * @param uuid_gen Ссылка на объект UUIDGenerator для генерации UUID.
 * @param redis Ссылка на объект sw::redis::Redis для взаимодействия с Redis.
 */
TokenGenerator::TokenGenerator(UUIDGenerator& uuid_gen, sw::redis::Redis& redis)
    : uuid_gen_(uuid_gen), 
      redis_(redis) 
{}