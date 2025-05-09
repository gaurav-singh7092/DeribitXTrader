#!/bin/bash

# Script to build and run all tests for the DERIBitXTrader project

# Set the script to exit on error
set -e

# Print a message with a colored header
print_header() {
    echo -e "\033[1;34m==== $1 ====\033[0m"
}

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    print_header "Creating build directory"
    mkdir -p build
fi

# Navigate to build directory
cd build

# Configure the project with CMake
print_header "Configuring project with CMake"
cmake ..

# Build the project
print_header "Building project"
make

# Run unit tests
print_header "Running unit tests"
cd tests
./unit_tests

# Run integration tests
print_header "Running integration tests"
./integration_tests

# Run performance tests
print_header "Running performance tests"
./performance_tests

# Return to the project root
cd ../..

print_header "All tests completed successfully!"
