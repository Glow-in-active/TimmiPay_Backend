#include "finance_app.h"
#include "../server/db_init/db_init.h"
#include "../server/server.h"
#include <iostream>
#include <string>

int run_finance_app() {
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