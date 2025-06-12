#pragma once

#include <crow.h>
#include <crow/middlewares/cors.h>
#include "../../auth/user_verify_http/session_start/session_start.h"
#include "../../auth/user_verify_http/session_hold/session_hold.h"
#include "../../balance_viewer/logic/balance_viewer.h"
#include <functional>
#include <memory>

crow::App<crow::CORSHandler>& create_crow_app(
    SessionStart& session_start_handler,
    SessionHold& session_hold_handler,
    BalanceViewer& balance_viewer_handler
);