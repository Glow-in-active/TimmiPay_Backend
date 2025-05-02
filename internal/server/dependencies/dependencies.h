#pragma once

#include "../../auth/user_verify/verification/user_verify.h"
#include "../../auth/user_verify_http/session_start/session_start.h"
#include "../../auth/user_verify_http/session_hold/session_hold.h"
#include "../db_init/db_init.h"

struct Dependencies {
    UserVerifier user_verifier;
    SessionStart session_start_handler;
    SessionHold session_hold_handler;
};

Dependencies initialize_dependencies(DBConnections& db);
