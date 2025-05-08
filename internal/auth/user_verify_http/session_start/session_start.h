#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include "../../user_verify/verification/user_verify.h"

using json = nlohmann::json;

class SessionStart {
public:
    explicit SessionStart(UserVerifier& user_verifier);
    
    json HandleRequest(const json& request_data);

private:
    UserVerifier& user_verifier_;
};