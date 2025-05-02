#pragma once

#include <crow.h>
#include <crow/middlewares/cors.h> 
#include "../../auth/user_verify_http/session_start/session_start.h"
#include "../../auth/user_verify_http/session_hold/session_hold.h"

void register_routes(crow::App<crow::CORSHandler>& app, SessionStart& session_start_handler, SessionHold& session_hold_handler);