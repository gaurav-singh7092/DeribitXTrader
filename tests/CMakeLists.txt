cmake_minimum_required(VERSION 3.10)

# Include FetchContent for downloading dependencies
include(FetchContent)

# Fetch Google Test
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${googletest_SOURCE_DIR}/googletest/include
    ${fmt_SOURCE_DIR}/include
    ${ixwebsocket_SOURCE_DIR}
    ${NLOHMANN_JSON_INCLUDE_DIRS}
)

# Add test executables
add_executable(unit_tests
    unit/test_market_api.cpp
    unit/test_socket_client.cpp
    unit/test_performance_monitor.cpp
    unit/test_utility.cpp
    unit/test_json_parser.cpp
    unit/test_credentials.cpp
    # Add more unit test files as needed
)

add_executable(integration_tests
    integration/test_deribit_api.cpp
    integration/test_websocket_connection.cpp
    # Add more integration test files as needed
)

add_executable(performance_tests
    performance/test_json_performance.cpp
    performance/test_websocket_performance.cpp
    performance/test_market_api_performance.cpp
    # Add more performance test files as needed
)

# Link libraries
target_link_libraries(unit_tests
    gtest
    gtest_main
    deribit_trader_lib
)

target_link_libraries(integration_tests
    gtest
    gtest_main
    deribit_trader_lib
)

target_link_libraries(performance_tests
    gtest
    gtest_main
    deribit_trader_lib
    # Add benchmark library
)

# Register tests
enable_testing()
add_test(NAME UnitTests COMMAND unit_tests)
add_test(NAME IntegrationTests COMMAND integration_tests)
add_test(NAME PerformanceTests COMMAND performance_tests)
