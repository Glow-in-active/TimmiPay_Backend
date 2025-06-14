#ifndef TOKEN_GENERATOR_H
#define TOKEN_GENERATOR_H

#include <sw/redis++/redis++.h>

#include "../../../../uuid_generator/uuid_generator.h"
#include "../../models/user.h"

/**
 * @brief Класс для генерации и управления токенами сессий.
 *
 * Использует UUIDGenerator для создания уникальных токенов и Redis для их
 * хранения.
 */
class TokenGenerator {
 public:
  /**
   * @brief Конструктор класса TokenGenerator.
   *
   * @param uuid_gen Ссылка на объект UUIDGenerator для генерации UUID.
   * @param redis Ссылка на объект sw::redis::Redis для взаимодействия с Redis.
   */
  TokenGenerator(UUIDGenerator& uuid_gen, sw::redis::Redis& redis);

  /**
   * @brief Генерирует новый токен для пользователя.
   *
   * @param user Объект User, для которого генерируется токен.
   * @return Сгенерированный токен.
   */
  std::string GenerateToken(const User& user);

 private:
  UUIDGenerator& uuid_gen_;
  sw::redis::Redis& redis_;
};

#endif