cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0048 NEW)

# Set project details
project(DeribitTrader VERSION 1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include external dependencies
include(FetchContent)

# Fetch IXWebSocket library
FetchContent_Declare(
    ixwebsocket
    GIT_REPOSITORY https://github.com/machinezone/IXWebSocket.git
    GIT_TAG v11.4.4
)
set(USE_TLS ON CACHE BOOL "Enable TLS support" FORCE)
set(USE_ZLIB OFF CACHE BOOL "Enable zlib support" FORCE)
FetchContent_MakeAvailable(ixwebsocket)

# Fetch fmt library
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt
    GIT_TAG e69e5f977d458f2650bb346dadf2ad30c5320281
)
FetchContent_MakeAvailable(fmt)

# Find required packages
find_package(OpenSSL REQUIRED)

# Find nlohmann-json from Homebrew installation
# This adds proper include paths for the JSON library
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(NLOHMANN_JSON nlohmann_json)
endif()

# If pkg-config couldn't find it, try the direct path for macOS Homebrew
if(NOT NLOHMANN_JSON_FOUND AND APPLE)
    set(NLOHMANN_JSON_INCLUDE_DIRS "/opt/homebrew/opt/nlohmann-json/include")
    if(NOT EXISTS "${NLOHMANN_JSON_INCLUDE_DIRS}/nlohmann/json.hpp")
        message(WARNING "nlohmann/json.hpp not found at expected Homebrew location")
    endif()
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Define source files
set(SOURCES
    src/security/credentials.cpp
    src/exchange_interface/market_api.cpp
    src/helpers/utility.cpp
    src/network/socket_client.cpp
    src/performance/monitor.cpp
)

# Create a library for the common code
add_library(deribit_trader_lib STATIC ${SOURCES})

# Add include directories for the library
target_include_directories(deribit_trader_lib
    PRIVATE
        ${OPENSSL_INCLUDE_DIR}
        ${fmt_SOURCE_DIR}/include
        ${NLOHMANN_JSON_INCLUDE_DIRS}
        ${ixwebsocket_SOURCE_DIR}
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)

# Link required libraries to the library
target_link_libraries(deribit_trader_lib
    PRIVATE
        OpenSSL::SSL
        OpenSSL::Crypto
        fmt::fmt
        readline
        ixwebsocket
)

# Add executable and its source files
add_executable(trade_x_deribit
    src/main.cpp
)

# Add include directories
target_include_directories(trade_x_deribit
    PRIVATE
        ${OPENSSL_INCLUDE_DIR}
        ${fmt_SOURCE_DIR}/include
        ${NLOHMANN_JSON_INCLUDE_DIRS}
        ${ixwebsocket_SOURCE_DIR}
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include # Include your project headers
)

# Link required libraries
target_link_libraries(trade_x_deribit
    PRIVATE
        deribit_trader_lib
)

# Use platform-specific linker flags
if(UNIX AND NOT APPLE)
    set_target_properties(trade_x_deribit PROPERTIES
        LINK_FLAGS "-Wl,--export-dynamic"
    )
endif()

# Debugging information
message(STATUS "OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message(STATUS "nlohmann-json include dir: ${NLOHMANN_JSON_INCLUDE_DIRS}")

# Add tests
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
