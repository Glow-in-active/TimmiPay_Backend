#include "uuid_generator.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

class UUIDGenerator::Impl {
public:
    boost::uuids::random_generator generator;
};

UUIDGenerator::UUIDGenerator() : impl(new Impl()) {}

std::string UUIDGenerator::generateUUID() {
    boost::uuids::uuid uuid = impl->generator();
    return boost::uuids::to_string(uuid);
}