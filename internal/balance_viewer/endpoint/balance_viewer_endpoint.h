#pragma once

#include <crow.h>
#include "../logic/balance_viewer.h"

std::function<crow::response(const crow::request&)> create_balance_viewer_handler(BalanceViewer& handler);