cmake_minimum_required(VERSION 3.20)

project(peerchat
    VERSION 0.1.0
    DESCRIPTION "Peer-to-peer decentralized chat application"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- Options ---
option(PEERCHAT_BUILD_TESTS "Build tests" ON)

# --- Compiler warnings ---
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# --- Dependencies via FetchContent ---
include(FetchContent)

# asio (standalone, no Boost)
FetchContent_Declare(
    asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG asio-1-30-2
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(asio)

add_library(asio INTERFACE)
target_include_directories(asio INTERFACE ${asio_SOURCE_DIR}/asio/include)
target_compile_definitions(asio INTERFACE ASIO_STANDALONE)
if(WIN32)
    target_link_libraries(asio INTERFACE ws2_32 wsock32)
endif()

# spdlog
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.14.1
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(spdlog)

# nlohmann/json
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

# cpp-httplib (header-only HTTP/HTTPS client)
FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.15.3
    GIT_SHALLOW TRUE
)
set(HTTPLIB_REQUIRE_OPENSSL ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(httplib)

# --- Find system libsodium ---
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(SODIUM IMPORTED_TARGET libsodium)
endif()

if(NOT SODIUM_FOUND)
    find_library(SODIUM_LIBRARY NAMES sodium libsodium)
    find_path(SODIUM_INCLUDE_DIR NAMES sodium.h)
    if(SODIUM_LIBRARY AND SODIUM_INCLUDE_DIR)
        add_library(sodium_lib INTERFACE)
        target_include_directories(sodium_lib INTERFACE ${SODIUM_INCLUDE_DIR})
        target_link_libraries(sodium_lib INTERFACE ${SODIUM_LIBRARY})
    else()
        message(WARNING "libsodium not found. Encryption features will be disabled.")
        set(PEERCHAT_NO_SODIUM TRUE)
    endif()
endif()

# --- Main library ---
add_library(peerchat_lib STATIC
    src/version.cpp
    src/message.cpp
    src/framing.cpp
    src/identity.cpp
    src/connection.cpp
    src/server.cpp
    src/client.cpp
    src/peer_manager.cpp
    src/cli.cpp
    src/app.cpp
    src/updater.cpp
)

target_include_directories(peerchat_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(peerchat_lib PUBLIC
    asio
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    httplib::httplib
)

if(SODIUM_FOUND)
    target_link_libraries(peerchat_lib PUBLIC PkgConfig::SODIUM)
    target_compile_definitions(peerchat_lib PUBLIC PEERCHAT_HAS_SODIUM=1)
elseif(TARGET sodium_lib)
    target_link_libraries(peerchat_lib PUBLIC sodium_lib)
    target_compile_definitions(peerchat_lib PUBLIC PEERCHAT_HAS_SODIUM=1)
endif()

# Platform-specific threading
if(NOT WIN32)
    find_package(Threads REQUIRED)
    target_link_libraries(peerchat_lib PUBLIC Threads::Threads)
endif()

# --- Main executable ---
add_executable(peerchat src/main.cpp)
target_link_libraries(peerchat PRIVATE peerchat_lib)

# --- Tests ---
if(PEERCHAT_BUILD_TESTS)
    enable_testing()

    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
        GIT_SHALLOW TRUE
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_executable(peerchat_tests
        tests/test_main.cpp
        tests/test_version.cpp
        tests/test_message.cpp
        tests/test_framing.cpp
        tests/test_identity.cpp
        tests/test_connection.cpp
    )

    target_link_libraries(peerchat_tests PRIVATE
        peerchat_lib
        GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(peerchat_tests)
endif()

# --- Install ---
install(TARGETS peerchat DESTINATION bin)
