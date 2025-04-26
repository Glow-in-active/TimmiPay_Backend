#pragma once

#include <string>

struct Config {
    std::string host;
    int port;
    std::string user;
    std::string password;
    std::string dbname;
    std::string sslmode;
};

Config load_config(const std::string& filename);