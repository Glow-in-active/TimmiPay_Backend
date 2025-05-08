#pragma once

#include <crow.h>
#include "../../session_start/session_start.h"

std::function<crow::response(const crow::request&)> create_session_auth_handler(SessionStart& handler);