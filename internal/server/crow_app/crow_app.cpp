#include "crow_app.h"
#include "../api_methods/api_methods.h"

crow::App<crow::CORSHandler>& create_crow_app(
    SessionStart& session_start_handler,
    SessionHold& session_hold_handler
) {
    static crow::App<crow::CORSHandler> app;
    
    auto& cors = app.get_middleware<crow::CORSHandler>();
    cors.global()
        .headers("Content-Type", "Authorization")
        .methods("POST"_method)
        .origin("*");

    register_routes(app, session_start_handler, session_hold_handler);
    
    return app;
}