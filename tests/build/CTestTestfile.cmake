# CMake generated Testfile for 
# Source directory: /Users/gauravsingh07/Desktop/projects/DERIBitXTrader/tests
# Build directory: /Users/gauravsingh07/Desktop/projects/DERIBitXTrader/tests/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(UnitTests "/Users/gauravsingh07/Desktop/projects/DERIBitXTrader/tests/build/unit_tests")
set_tests_properties(UnitTests PROPERTIES  _BACKTRACE_TRIPLES "/Users/gauravsingh07/Desktop/projects/DERIBitXTrader/tests/CMakeLists.txt;69;add_test;/Users/gauravsingh07/Desktop/projects/DERIBitXTrader/tests/CMakeLists.txt;0;")
add_test(IntegrationTests "/Users/gauravsingh07/Desktop/projects/DERIBitXTrader/tests/build/integration_tests")
set_tests_properties(IntegrationTests PROPERTIES  _BACKTRACE_TRIPLES "/Users/gauravsingh07/Desktop/projects/DERIBitXTrader/tests/CMakeLists.txt;70;add_test;/Users/gauravsingh07/Desktop/projects/DERIBitXTrader/tests/CMakeLists.txt;0;")
add_test(PerformanceTests "/Users/gauravsingh07/Desktop/projects/DERIBitXTrader/tests/build/performance_tests")
set_tests_properties(PerformanceTests PROPERTIES  _BACKTRACE_TRIPLES "/Users/gauravsingh07/Desktop/projects/DERIBitXTrader/tests/CMakeLists.txt;71;add_test;/Users/gauravsingh07/Desktop/projects/DERIBitXTrader/tests/CMakeLists.txt;0;")
subdirs("_deps/googletest-build")
