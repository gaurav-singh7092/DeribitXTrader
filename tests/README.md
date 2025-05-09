# DERIBitXTrader Tests

This directory contains unit tests and integration tests for the DERIBitXTrader project.

## Test Structure

The tests are organized into three main categories:

1. **Unit Tests**: Tests for individual components in isolation
   - Located in `tests/unit/`
   - Test files are named `test_*.cpp`

2. **Integration Tests**: Tests for interactions between components
   - Located in `tests/integration/`
   - Test files are named `test_*.cpp`

3. **Performance Tests**: Tests for measuring performance of critical components
   - Located in `tests/performance/`
   - Test files are named `test_*_performance.cpp`
   - Measure execution time for various operations
   - Help identify performance bottlenecks

## Running Tests

### Using the Test Script

The easiest way to run all tests is to use the provided script:

```bash
./scripts/run_tests.sh
```

This script will:
1. Create a build directory if it doesn't exist
2. Configure the project with CMake
3. Build the project
4. Run all unit tests
5. Run all integration tests

### Running Tests Manually

If you prefer to run tests manually, follow these steps:

1. Create a build directory and navigate to it:
   ```bash
   mkdir -p build && cd build
   ```

2. Configure the project with CMake:
   ```bash
   cmake ..
   ```

3. Build the project:
   ```bash
   make
   ```

4. Run the unit tests:
   ```bash
   cd tests
   ./unit_tests
   ```

5. Run the integration tests:
   ```bash
   ./integration_tests
   ```

6. Run the performance tests:
   ```bash
   ./performance_tests
   ```

## Adding New Tests

### Adding a Unit Test

1. Create a new test file in `tests/unit/` named `test_<component>.cpp`
2. Add the test file to the `unit_tests` executable in `tests/CMakeLists.txt`
3. Write your tests using the Google Test framework

Example:
```cpp
#include <gtest/gtest.h>
#include "your_component.h"

TEST(YourComponentTest, TestName) {
    // Test code here
    EXPECT_TRUE(true);
}
```

### Adding an Integration Test

1. Create a new test file in `tests/integration/` named `test_<feature>.cpp`
2. Add the test file to the `integration_tests` executable in `tests/CMakeLists.txt`
3. Write your tests using the Google Test framework

### Adding a Performance Test

1. Create a new test file in `tests/performance/` named `test_<component>_performance.cpp`
2. Add the test file to the `performance_tests` executable in `tests/CMakeLists.txt`
3. Use the `PerformanceTimer` class to measure execution time
4. Write your tests using the Google Test framework

Example:
```cpp
#include <gtest/gtest.h>
#include <chrono>
#include <iostream>

class PerformanceTimer {
private:
    std::chrono::high_resolution_clock::time_point start_time;
    std::string operation_name;
    int iterations;

public:
    PerformanceTimer(const std::string& name, int iters)
        : operation_name(name), iterations(iters) {
        start_time = std::chrono::high_resolution_clock::now();
    }

    ~PerformanceTimer() {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
        double avg_duration = static_cast<double>(duration) / iterations;

        std::cout << "PERFORMANCE [" << operation_name << "]: "
                  << duration << " µs total, "
                  << avg_duration << " µs per operation ("
                  << iterations << " iterations)" << std::endl;
    }
};

TEST(YourPerformanceTest, OperationPerformance) {
    const int iterations = 1000;
    PerformanceTimer timer("Your Operation", iterations);

    for (int i = 0; i < iterations; i++) {
        // Code to measure
        // ...
    }
}
```

## Test Dependencies

The tests use the following dependencies:

- **Google Test**: For test assertions and test runner
- **IXWebSocket**: For WebSocket communication tests
- **fmt**: For formatting output
- **nlohmann/json**: For JSON parsing and manipulation

These dependencies are automatically fetched and built by CMake.
