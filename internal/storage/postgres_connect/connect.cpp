#include "connect.h"
#include <stdexcept>
#include <string>

pqxx::connection connect_to_database(const Config& config) {
    try {
        std::string connection_params = 
            "host=" + config.host +
            " port=" + std::to_string(config.port) +
            " user=" + config.user +
            " password=" + config.password +
            " dbname=" + config.dbname +
            " sslmode=" + config.sslmode;

        return pqxx::connection(connection_params);
    }
    catch (const pqxx::sql_error& e) {
        throw std::runtime_error(
            "Database error: " + std::string(e.what()) +
            "\nQuery was: " + e.query()
        );
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Connection failed: " + std::string(e.what()));
    }
}