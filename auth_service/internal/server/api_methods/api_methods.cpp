#include "api_methods.h"
#include "../../auth/user_verify_http/endpoints/session_auth_endpoint/session_auth_endpoint.h"
#include "../../auth/user_verify_http/endpoints/session_refresh_endpoint/session_refresh_endpoint.h"

void register_routes(crow::App<crow::CORSHandler>& app, SessionStart& session_start_handler, SessionHold& session_hold_handler) {
    CROW_ROUTE(app, "/session_start").methods("POST"_method)(
        create_session_auth_handler(session_start_handler)
    );

    CROW_ROUTE(app, "/session_refresh").methods("POST"_method)(
        create_session_refresh_handler(session_hold_handler)
    );
}




