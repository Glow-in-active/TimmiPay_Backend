#include "start_server.h"
#include "../api_methods/api_methods.h"
#include "../crow_app/crow_app.h"
#include "../db_init/db_init.h"
#include <crow/middlewares/cors.h>
#include <iostream>

void start_server(Dependencies& deps) {
    try {
        auto& app = create_crow_app(deps.session_start_handler, deps.session_hold_handler);

        app.port(8080).multithreaded().run();

    } catch (const pqxx::sql_error& e) {
        std::cerr << "PostgreSQL error: " << e.what() << std::endl;
    } catch (const sw::redis::Error& e) {
        std::cerr << "Redis error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
    }
}