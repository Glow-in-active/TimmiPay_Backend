#include "api_methods.h"
#include "../../auth/user_verify_http/endpoints/session_auth_endpoint/session_auth_endpoint.h"
#include "../../auth/user_verify_http/endpoints/session_refresh_endpoint/session_refresh_endpoint.h"
#include "../../balance_viewer/endpoint/balance_viewer_endpoint.h"

void register_routes(
    crow::App<crow::CORSHandler>& app,
    SessionStart& session_start_handler,
    SessionHold& session_hold_handler,
    BalanceViewer& balance_viewer_handler
) {
    CROW_ROUTE(app, "/session_start").methods("POST"_method)(
        create_session_auth_handler(session_start_handler)
    );

    CROW_ROUTE(app, "/session_refresh").methods("POST"_method)(
        create_session_refresh_handler(session_hold_handler)
    );

    CROW_ROUTE(app, "/balance_viewer").methods("POST"_method)(
        create_balance_viewer_handler(balance_viewer_handler)
    );
}
