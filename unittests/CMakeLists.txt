cmake_minimum_required(VERSION 3.21)
set(VCPKG_TARGET_TRIPLET x64-windows-static)
project(qtadstestunits VERSION "3.4" LANGUAGES C CXX)

# Use a toolmake chain file
set(CMAKE_TOOLCHAIN_FILE on)

# Setup the C and C++ language versions, set them as required, and disable compiler language extensions
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GTest REQUIRED)

enable_testing()
add_executable(Setup unittests/Setup.cpp)
target_link_libraries(Setup GTest::GTest GTest::Main)

if(GCC AND (TEST_architecture_arch STREQUAL "i386") AND WIN32)
    target_compile_options(qtads
        -march=i686
        -mtune=generic
    )
endif()

if(GCC AND (TEST_architecture_arch STREQUAL "x86_64") AND WIN32 AND NOT (TEST_architecture_arch STREQUAL "i386"))
    target_compile_options(qtads
        -march=x86-64
        -mtune=generic
    )
endif()
if(CLANG OR GCC)
    target_compile_definitions(Setup PRIVATE
        _DEFAULT_SOURCE
        _SVID_SOURCE
    )

    target_compile_options(Setup
        -fno-sized-deallocation
        -fno-strict-aliasing
    )
endif()



gtest_discover_tests(Setup)