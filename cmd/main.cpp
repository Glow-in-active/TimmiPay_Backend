#include "../internal/uuid_generator/uuid_generator.h"
#include <iostream>

int main() {
    UUIDGenerator generator;
    std::string uuid = generator.generateUUID();
    std::cout << uuid;
    return 0;
}