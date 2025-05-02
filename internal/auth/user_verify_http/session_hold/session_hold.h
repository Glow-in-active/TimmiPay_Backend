#ifndef SESSION_HOLD_H
#define SESSION_HOLD_H

#include <nlohmann/json.hpp>
#include <sw/redis++/redis++.h>

class SessionHold {
public:
    explicit SessionHold(sw::redis::Redis& redis);
    nlohmann::json HandleRequest(const nlohmann::json& request_data);
    sw::redis::Redis& redis_;
};

#endif