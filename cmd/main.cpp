#include "../internal/storage/config/config.h"
#include "../internal/storage/postgres_connect/connect.h"
#include <pqxx/pqxx>
#include <iostream>

int main() {
    try {
        Config config = load_config("postgres-config.json");
        pqxx::connection conn = connect_to_database(config);
        
        pqxx::work txn(conn);
        pqxx::result result = txn.exec("SELECT version()");
        txn.commit();
        
        std::cout << "я хочу какать" << " " << result[0][0] << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}