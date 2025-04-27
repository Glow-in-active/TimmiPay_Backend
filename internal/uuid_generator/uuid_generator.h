#ifndef UUID_GENERATOR_H
#define UUID_GENERATOR_H

#include <string>

class UUIDGenerator {
public:
    UUIDGenerator();
    std::string generateUUID();

private:
    class Impl;
    Impl* impl;
};

#endif