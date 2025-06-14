#include "finance_app.h"

#include <iostream>
#include <string>

#include "../server/db_init/db_init.h"
#include "../server/server.h"

/**
 * @brief Запускает финансовое приложение.
 *
 * Инициализирует соединения с базами данных, создает и запускает финансовый
 * сервер.
 *
 * @return 0 в случае успешного выполнения, 1 в случае ошибки.
 */
int run_finance_app() {
  try {
    DBConnections db = initialize_databases();
    int port = 8181;

    FinanceServer server(db.postgres, db.redis);
    std::cout << "Starting finance server on port " << port << std::endl;
    server.run(port);
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}