cmake_minimum_required(VERSION 3.10)
project(myapp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Vcpkg toolchain (убедитесь, что путь правильный!)
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")

find_package(nlohmann_json REQUIRED)
find_package(GTest REQUIRED)
find_package(libpqxx REQUIRED)  # Используем современный вариант

# Общая библиотека
add_library(app_lib
    internal/storage/config/config.cpp
    internal/storage/postgres_connect/connect.cpp
    internal/storage/user_verify/user_verify.cpp
    internal/user_verify/verification/user_verify.cpp
    internal/user_verify/token_generator/token_generator.cpp
)

target_include_directories(app_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/storage/config
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/storage/postgres_connect
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/storage/user_verify
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/user_verify/verification
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/user_verify/token_generator
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/models

)

target_link_libraries(app_lib PUBLIC
    nlohmann_json::nlohmann_json
    libpqxx::pqxx
)

# Основное приложение
add_executable(myapp cmd/main.cpp)
target_link_libraries(myapp PRIVATE app_lib)

# --- Один общий исполняемый файл для всех тестов ---

add_executable(all_tests
    internal/storage/config/config_test.cpp
    internal/storage/postgres_connect/connect_test.cpp
)

target_include_directories(all_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/storage/config
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/storage/postgres_connect
)

target_link_libraries(all_tests PRIVATE
    app_lib
    GTest::gtest_main
    libpqxx::pqxx
)

# Включаем автоматический запуск тестов через CTest
enable_testing()
add_test(NAME AllTests COMMAND all_tests)
