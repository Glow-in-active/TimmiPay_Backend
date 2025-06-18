#include "crow_app.h"

#include "../api_methods/api_methods.h"
#include "../../auth/user_verify_http/endpoints/session_auth_endpoint/session_auth_endpoint.h"
#include "../../auth/user_verify_http/endpoints/session_refresh_endpoint/session_refresh_endpoint.h"
#include "../../auth/user_verify_http/endpoints/registration_endpoint/registration_endpoint.h"

/**
 * @brief Создает и настраивает экземпляр crow::App с обработкой CORS.
 *
 * @param deps Объект Dependencies, содержащий все необходимые обработчики.
 * @return Ссылка на настроенный объект crow::App.
 */
crow::App<crow::CORSHandler>& create_crow_app(
    Dependencies& deps) {
  static crow::App<crow::CORSHandler> app;

  // Enable CORS for all routes
  auto& cors = app.get_middleware<crow::CORSHandler>();
  cors.global()
      .headers("Content-Type", "Authorization")
      .methods("POST"_method)
      .origin("*");

  // Define API endpoints
  CROW_ROUTE(app, "/auth")
      .methods("POST"_method)(create_session_auth_handler(deps.session_start_handler));
  CROW_ROUTE(app, "/refresh")
      .methods("POST"_method)(create_session_refresh_handler(deps.session_hold_handler));
  CROW_ROUTE(app, "/register")
      .methods("POST"_method)(create_registration_handler(deps.user_verifier.GetUserStorage()));

  return app;
}