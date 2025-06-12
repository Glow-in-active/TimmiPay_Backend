#include "dependencies.h"

Dependencies initialize_dependencies(DBConnections& db) {
    UserVerifier user_verifier(db.postgres, db.redis);
    SessionStart session_start_handler(user_verifier);
    SessionHold session_hold_handler(db.redis);

    return { user_verifier, session_start_handler, session_hold_handler };
}
