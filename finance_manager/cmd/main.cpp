#include "../internal/server/db_init/db_init.h"
#include "../internal/server/server.cpp"
#include <iostream>
#include <string>

int main() {
    try {
        DBConnections db = initialize_databases();
        int port = 8181;

        FinanceServer server(db.postgres, db.redis);
        std::cout << "Starting finance server on port " << port << std::endl;
        server.run(port);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
} 