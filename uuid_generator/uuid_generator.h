#ifndef UUID_GENERATOR_H
#define UUID_GENERATOR_H

#include <string>

class UUIDGenerator {
 public:
  UUIDGenerator() = default;
  UUIDGenerator(const UUIDGenerator&) = default;
  UUIDGenerator& operator=(const UUIDGenerator&) = default;
  ~UUIDGenerator() = default;

  /**
   * @brief Генерирует новый UUID.
   *
   * @return Строка, представляющая сгенерированный UUID.
   */
  std::string generateUUID();
};

#endif
