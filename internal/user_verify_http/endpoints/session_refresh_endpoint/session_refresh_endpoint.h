#pragma once

#include <crow.h>
#include "../../session_hold/session_hold.h"

std::function<crow::response(const crow::request&)> create_session_refresh_handler(SessionHold& handler);