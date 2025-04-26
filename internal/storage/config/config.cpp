#include "config.h"
#include <fstream>
#include <nlohmann/json.hpp>

Config load_config(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open config file: " + filename);
    }

    nlohmann::json data = nlohmann::json::parse(file);

    return Config {
        .host = data["host"].get<std::string>(),
        .port = data["port"].get<int>(),
        .user = data["user"].get<std::string>(),
        .password = data["password"].get<std::string>(),
        .dbname = data["dbname"].get<std::string>(),
        .sslmode = data["sslmode"].get<std::string>()
    };
}