#include "../internal/server/db_init/db_init.h"
#include "../internal/server/dependencies/dependencies.h"
#include "../internal/server/start_server/start_server.h"
#include <iostream>

int main() {
    DBConnections db = initialize_databases();

    Dependencies deps = initialize_dependencies(db);

    start_server(deps);

    return 0;
}
