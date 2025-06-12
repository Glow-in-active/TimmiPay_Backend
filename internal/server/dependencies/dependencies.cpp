#include "dependencies.h"
#include "balance_viewer.h"

Dependencies initialize_dependencies(DBConnections& db) {
    UserVerifier user_verifier(db.postgres, db.redis);
    SessionStart session_start_handler(user_verifier);
    SessionHold session_hold_handler(db.redis);
    BalanceStorage balance_storage(db.postgres);
    BalanceViewer balance_viewer_hander(db.redis, balance_storage);

    return {
        user_verifier,
        session_start_handler,
        session_hold_handler,
        balance_viewer_hander,
    };
}
