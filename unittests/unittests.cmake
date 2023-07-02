# #############################################################################
# Unit Tests for the DAP implementation
# ----------------------------------------------------------
# Include the google test package
find_package(GTest REQUIRED)
# set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# Enable the unit tests
enable_testing()
# Add the files containing the unit tests
add_executable(setup unittests/setup.cpp)
# Add the google test libraries. The GTest::Main defines a main method,
target_link_libraries(setup GTest::GTest GTest::Main)
# Instruct cmake to discover the gtests so they will show up in Visual Studio's test explorer
gtest_discover_tests(setup)