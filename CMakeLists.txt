cmake_minimum_required(VERSION 3.10)
project(myapp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Vcpkg toolchain (убедитесь, что путь правильный!)
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")

find_package(nlohmann_json REQUIRED)
find_package(GTest REQUIRED)
find_package(libpqxx REQUIRED)  # Используем современный вариант
find_package(Boost REQUIRED COMPONENTS uuid)
find_package(redis++ CONFIG REQUIRED)

# Общая библиотека
add_library(app_lib
    internal/storage/config/config.cpp
    internal/storage/postgres_connect/connect.cpp
    internal/storage/user_verify/auth/user_verify.cpp
    internal/auth/user_verify/verification/user_verify.cpp
    internal/auth/user_verify/token_generator/token_generator.cpp
    internal/uuid_generator/uuid_generator.cpp
    internal/storage/redis_config/config_redis.cpp
    internal/storage/redis_connect/connect_redis.cpp
    internal/storage/user_verify/redis_set/redis_set_token.cpp
    internal/auth/user_verify_http/session_start/session_start.cpp
    internal/auth/user_verify_http/session_hold/session_hold.cpp
    internal/auth/user_verify_http/endpoints/session_auth_endpoint/session_auth_endpoint.cpp
    internal/auth/user_verify_http/endpoints/session_refresh_endpoint/session_refresh_endpoint.cpp
)

target_include_directories(app_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/storage/config
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/storage/postgres_connect
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/storage/user_verify/auth
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/auth/user_verify/verification
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/auth/user_verify/token_generator
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/models
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/uuid_generator
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/storage/redis_config
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/storage/redis_connect
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/storage/user_verify/redis_set
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/auth/user_verify_http/session_start
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/auth/user_verify_http/session_hold
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/auth/user_verify_http/endpoints/session_auth_endpoint
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/auth/user_verify_http/endpoints/session_refresh_endpoint
)

target_link_libraries(app_lib PUBLIC
    nlohmann_json::nlohmann_json
    libpqxx::pqxx
    Boost::uuid
    redis++::redis++_static
)
# Основное приложение
add_executable(myapp cmd/main.cpp)
target_link_libraries(myapp PRIVATE app_lib)

# --- Один общий исполняемый файл для всех тестов ---

add_executable(all_tests
    internal/storage/config/config_test.cpp
    internal/storage/postgres_connect/connect_test.cpp
    internal/auth/user_verify/verification/user_verify_test.cpp
    internal/uuid_generator/uuid_generator_test.cpp
    internal/storage/redis_config/config_redis_test.cpp
    internal/storage/redis_connect/connect_redis_test.cpp
    internal/storage/user_verify/auth/user_verify_test.cpp
    internal/storage/user_verify/redis_set/redis_set_token_test.cpp
    internal/auth/user_verify/token_generator/token_generator_test.cpp
    internal/auth/user_verify_http/session_start/session_start_test.cpp
    internal/auth/user_verify_http/session_hold/session_hold_test.cpp
    internal/auth/user_verify_http/endpoints/session_auth_endpoint/session_auth_endpoint_test.cpp
    internal/auth/user_verify_http/endpoints/session_refresh_endpoint/session_refresh_endpoint_test.cpp
)

target_include_directories(all_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/storage/config
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/storage/postgres_connect
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/auth/user_verify/verification
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/storage/redis_config
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/storage/redis_connect
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/storage/user_verify/auth
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/storage/user_verify/redis_set
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/auth/user_verify/token_generator
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/auth/user_verify_http/session_start
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/auth/user_verify_http/session_hold
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/auth/user_verify_http/endpoints/session_auth_endpoint
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/auth/user_verify_http/endpoints/session_refresh_endpoint
)

target_link_libraries(all_tests PRIVATE
    app_lib
    GTest::gtest_main
    libpqxx::pqxx
)

# Включаем автоматический запуск тестов через CTest
enable_testing()
add_test(NAME AllTests COMMAND all_tests)
