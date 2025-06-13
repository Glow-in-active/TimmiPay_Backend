cmake_minimum_required(VERSION 3.10)
project(timmi_pay LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Vcpkg toolchain (убедитесь, что путь правильный!)
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")

find_package(nlohmann_json REQUIRED)
find_package(GTest REQUIRED)
find_package(libpqxx REQUIRED)  # Используем современный вариант
find_package(Boost REQUIRED COMPONENTS uuid)
find_package(redis++ CONFIG REQUIRED)
find_package(CURL REQUIRED)

# Общая библиотека
add_library(app_lib
    storage/config/config.cpp
    storage/postgres_connect/connect.cpp
    storage/user_verify/auth/user_verify.cpp
    storage/redis_config/config_redis.cpp
    storage/redis_connect/connect_redis.cpp
    storage/user_verify/redis_set/redis_set_token.cpp
    storage/session_verify/session_verify.cpp
    uuid_generator/uuid_generator.cpp
    auth_service/internal/auth/user_verify/verification/user_verify.cpp
    auth_service/internal/auth/user_verify/token_generator/token_generator.cpp
    auth_service/internal/auth/user_verify_http/session_start/session_start.cpp
    auth_service/internal/auth/user_verify_http/session_hold/session_hold.cpp
    auth_service/internal/auth/user_verify_http/endpoints/session_auth_endpoint/session_auth_endpoint.cpp
    auth_service/internal/auth/user_verify_http/endpoints/session_refresh_endpoint/session_refresh_endpoint.cpp
    auth_service/internal/server/api_methods/api_methods.cpp
    auth_service/internal/server/crow_app/crow_app.cpp
    auth_service/internal/server/db_init/db_init.cpp
    auth_service/internal/server/dependencies/dependencies.cpp
    auth_service/internal/server/start_server/start_server.cpp
    finance_manager/internal/server/server.cpp
    finance_manager/internal/finance/finance_service.cpp
)

target_include_directories(app_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/config
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/postgres_connect
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/user_verify/auth
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/redis_config
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/redis_connect
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/user_verify/redis_set
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/session_verify 
    ${CMAKE_CURRENT_SOURCE_DIR}/uuid_generator
    ${CMAKE_CURRENT_SOURCE_DIR}/auth_service/internal/auth/user_verify/verification
    ${CMAKE_CURRENT_SOURCE_DIR}/auth_service/internal/auth/user_verify/token_generator
    ${CMAKE_CURRENT_SOURCE_DIR}/auth_service/internal/models
    ${CMAKE_CURRENT_SOURCE_DIR}/auth_service/internal/auth/user_verify_http/session_start
    ${CMAKE_CURRENT_SOURCE_DIR}/auth_service/internal/auth/user_verify_http/session_hold
    ${CMAKE_CURRENT_SOURCE_DIR}/auth_service/internal/auth/user_verify_http/endpoints/session_auth_endpoint
    ${CMAKE_CURRENT_SOURCE_DIR}/auth_service/internal/auth/user_verify_http/endpoints/session_refresh_endpoint
    ${CMAKE_CURRENT_SOURCE_DIR}/auth_service/internal/server/api_methods
    ${CMAKE_CURRENT_SOURCE_DIR}/auth_service/internal/server/crow_app
    ${CMAKE_CURRENT_SOURCE_DIR}/auth_service/internal/server/db_init
    ${CMAKE_CURRENT_SOURCE_DIR}/auth_service/internal/server/dependencies
    ${CMAKE_CURRENT_SOURCE_DIR}/auth_service/internal/server/start_server
    ${CMAKE_CURRENT_SOURCE_DIR}/finance_manager/internal/server
    ${CMAKE_CURRENT_SOURCE_DIR}/finance_manager/internal/service
)

target_link_libraries(app_lib PUBLIC
    nlohmann_json::nlohmann_json
    libpqxx::pqxx
    Boost::uuid
    redis++::redis++_static
)

# Основные приложения
add_executable(auth_service auth_service/cmd/main.cpp)
target_link_libraries(auth_service PRIVATE app_lib)

add_executable(finance_manager finance_manager/cmd/main.cpp)
target_link_libraries(finance_manager PRIVATE app_lib)

# --- Один общий исполняемый файл для всех тестов ---

add_executable(all_tests
    storage/config/config_test.cpp
    storage/postgres_connect/connect_test.cpp
    storage/redis_config/config_redis_test.cpp
    storage/redis_connect/connect_redis_test.cpp
    storage/user_verify/auth/user_verify_test.cpp
    storage/user_verify/redis_set/redis_set_token_test.cpp
    uuid_generator/uuid_generator_test.cpp
    auth_service/internal/auth/user_verify/verification/user_verify_test.cpp
    auth_service/internal/auth/user_verify/token_generator/token_generator_test.cpp
    auth_service/internal/auth/user_verify_http/session_start/session_start_test.cpp
    auth_service/internal/auth/user_verify_http/session_hold/session_hold_test.cpp
    auth_service/internal/auth/user_verify_http/endpoints/session_auth_endpoint/session_auth_endpoint_test.cpp
    auth_service/internal/auth/user_verify_http/endpoints/session_refresh_endpoint/session_refresh_endpoint_test.cpp
    auth_service/internal/server/api_methods/api_methods_test.cpp
    auth_service/internal/server/crow_app/crow_app_test.cpp
    auth_service/internal/server/db_init/db_init_test.cpp
    auth_service/internal/server/dependencies/dependencies_test.cpp
    auth_service/internal/server/start_server/start_server_test.cpp
    finance_manager/internal/finance/finance_service_test.cpp
    #finance_manager/internal/server/finance_server_test.cpp
    #finance_manager/internal/models/transaction_test.cpp
    #finance_manager/internal/models/balance_test.cpp
)

target_include_directories(all_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/config
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/postgres_connect
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/redis_config
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/redis_connect
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/user_verify/auth
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/user_verify/redis_set
    ${CMAKE_CURRENT_SOURCE_DIR}/uuid_generator
    ${CMAKE_CURRENT_SOURCE_DIR}/auth_service/internal/auth/user_verify/verification
    ${CMAKE_CURRENT_SOURCE_DIR}/auth_service/internal/auth/user_verify/token_generator
    ${CMAKE_CURRENT_SOURCE_DIR}/auth_service/internal/auth/user_verify_http/session_start
    ${CMAKE_CURRENT_SOURCE_DIR}/auth_service/internal/auth/user_verify_http/session_hold
    ${CMAKE_CURRENT_SOURCE_DIR}/auth_service/internal/auth/user_verify_http/endpoints/session_auth_endpoint
    ${CMAKE_CURRENT_SOURCE_DIR}/auth_service/internal/auth/user_verify_http/endpoints/session_refresh_endpoint
    ${CMAKE_CURRENT_SOURCE_DIR}/auth_service/internal/server/api_methods
    ${CMAKE_CURRENT_SOURCE_DIR}/auth_service/internal/server/crow_app
    ${CMAKE_CURRENT_SOURCE_DIR}/auth_service/internal/server/db_init
    ${CMAKE_CURRENT_SOURCE_DIR}/auth_service/internal/server/dependencies
    ${CMAKE_CURRENT_SOURCE_DIR}/auth_service/internal/server/start_server
    ${CMAKE_CURRENT_SOURCE_DIR}/finance_manager/internal/finance
    #${CMAKE_CURRENT_SOURCE_DIR}/finance_manager/internal/server
    #${CMAKE_CURRENT_SOURCE_DIR}/finance_manager/internal/service
    #${CMAKE_CURRENT_SOURCE_DIR}/finance_manager/internal/models
)

target_link_libraries(all_tests PRIVATE
    app_lib
    GTest::gtest_main
    libpqxx::pqxx
    CURL::libcurl 
)

# Включаем автоматический запуск тестов через CTest
enable_testing()
add_test(NAME AllTests COMMAND all_tests)