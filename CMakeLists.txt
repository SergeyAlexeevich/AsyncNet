cmake_minimum_required(VERSION 3.5)

project(AsyncNet LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(AsyncNet
    src/main.cpp
    src/http_server/server.cpp
    src/handlers/handler.cpp
    src/logging/logger.cpp
    src/utils/utils.cpp
    src/tests/tests_assert.cpp
    src/tests/tests_handlers.cpp
    src/tests/tests_sessions.cpp
    )

target_include_directories(AsyncNet PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src)
