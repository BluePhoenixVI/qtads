# #############################################################################
# Unit Tests for the DAP implementation
# ----------------------------------------------------------
# Include the google test package
find_package(GTest REQUIRED)
# set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# Enable the unit tests
enable_testing()
# Add the files containing the unit tests
add_executable(UnitTests unittests/setup.cpp unittests/initializationtests.cpp unittests/ProcessHandler.cpp)
# Add the google test libraries. The GTest::Main defines a main method,
target_link_libraries(UnitTests GTest::GTest GTest::Main cppdap)
# Instruct cmake to discover the gtests so they will show up in Visual Studio's test explorer
gtest_discover_tests(UnitTests AUTO)
# The unit tests depend on qtads
add_dependencies(UnitTests qtads)



add_custom_target(BreakPointTest BYPRODUCTS BreakPointTest.t3 VERBATIM COMMAND t3make -o "${CMAKE_BINARY_DIR}/breakpointtest.t3" -FC -Fy obj -Fo obj -d -lib system -source ${CMAKE_CURRENT_SOURCE_DIR}/unittests/teststories/breakpointtest)

add_dependencies(UnitTests BreakPointTest)