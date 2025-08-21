cmake_minimum_required(VERSION 3.10)  # minimum CMake version
project(Custom-Programming-Language)  # project name

# =============================
# Source Files - src/
# =============================
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp)
list(REMOVE_ITEM SRC_FILES "${CMAKE_SOURCE_DIR}/src/main.cpp")  # exclude main.cpp

# Make .exe out of main.cpp
add_executable(Custom-Programming-Language main.cpp ${SRC_FILES})

# Require C++17
set_target_properties(
    Custom-Programming-Language PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

# =============================
# Headers - includes/
# =============================
target_include_directories(Custom-Programming-Language PRIVATE
    ${CMAKE_SOURCE_DIR}/includes
)


# Enable warnings
if (MSVC)
    target_compile_options(Custom-Programming-Language PRIVATE /W4 /permissive-)
else()
    target_compile_options(Custom-Programming-Language PRIVATE -Wall -Wextra -pedantic)
endif()


# =============================
# GoogleTest - tests/
# =============================
option(BUILD_TESTS "Build unit tests" OFF)

if (BUILD_TESTS)
    include(FetchContent)

    # Download GoogleTest at configure time
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/heads/main.zip
    )
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    # Collect test sources
    file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS tests/*.cpp)

    # Define test executable
    add_executable(unit_tests ${TEST_SOURCES})

    # Link with gtest and your project library
    target_link_libraries(unit_tests
        PRIVATE
        gtest_main
        Custom-Programming-Language
    )

    include(GoogleTest)
    gtest_discover_tests(unit_tests)
endif()


# =============================
# Usage
# =============================
# Normal build (no tests):
#   cd build
#   cmake ..
#   cmake --build .
#
# Build with tests:
#   cd build
#   cmake -DBUILD_TESTS=ON ..
#   cmake --build .
#   ctest