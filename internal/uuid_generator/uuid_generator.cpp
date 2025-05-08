#include "uuid_generator.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

std::string UUIDGenerator::generateUUID() {
    thread_local boost::uuids::random_generator generator;

    boost::uuids::uuid uuid = generator();
    return boost::uuids::to_string(uuid);
}
