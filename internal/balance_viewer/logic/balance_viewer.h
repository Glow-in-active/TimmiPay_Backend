#pragma once

#include <sw/redis++/redis++.h>
#include "users_balance.h"
#include <nlohmann/json.hpp>
#include <stdexcept>

class BalanceViewer {
public:
    BalanceViewer(sw::redis::Redis& redis, BalanceStorage& balance_storage);
    nlohmann::json HandleRequest(const nlohmann::json& request_data);

private:
    sw::redis::Redis& redis_;
    BalanceStorage& balance_storage_;
};