#include "uuid_generator.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

/**
 * @brief Генерирует новый UUID.
 *
 * Эта функция использует библиотеку Boost.UUID для создания универсального уникального идентификатора.
 *
 * @return Строка, представляющая сгенерированный UUID.
 */
std::string UUIDGenerator::generateUUID() {
    thread_local boost::uuids::random_generator generator;

    boost::uuids::uuid uuid = generator();
    return boost::uuids::to_string(uuid);
}
