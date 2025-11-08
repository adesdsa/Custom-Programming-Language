cmake_minimum_required(VERSION 3.14)  # FetchContent works best with >=3.14
project(Custom-Programming-Language)

# =============================
# C++ Settings
# =============================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# =============================
# Include FetchContent
# =============================
include(FetchContent)

# =============================
# Boost (iostreams)
# =============================
FetchContent_Declare(
    boost
    GIT_REPOSITORY https://github.com/boostorg/boost.git
    GIT_TAG boost-1.81.0
)
FetchContent_MakeAvailable(boost)

# =============================
# Source Files - src/
# =============================
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp)
list(REMOVE_ITEM SRC_FILES "${CMAKE_SOURCE_DIR}/src/main.cpp")

# =============================
# Library
# =============================
add_library(CPLib ${SRC_FILES})
target_include_directories(CPLib PUBLIC ${CMAKE_SOURCE_DIR}/includes)
target_link_libraries(CPLib PUBLIC Boost::iostreams)
target_compile_features(CPLib PUBLIC cxx_std_17)

# Enable warnings
if (MSVC)
    target_compile_options(CPLib PRIVATE /W4 /permissive-)
else()
    target_compile_options(CPLib PRIVATE -Wall -Wextra -pedantic)
endif()

# =============================
# Main executable
# =============================
add_executable(Custom-Programming-Language main.cpp)
target_link_libraries(Custom-Programming-Language PRIVATE CPLib)
target_compile_features(Custom-Programming-Language PUBLIC cxx_std_17)

# =============================
# GoogleTest - tests/
# =============================
option(BUILD_TESTS "Build unit tests" OFF)

if(BUILD_TESTS)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG main
    )
    set(FETCHCONTENT_QUIET FALSE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()
    file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS tests/*.cpp)

    add_executable(unit_tests ${TEST_SOURCES})
    target_link_libraries(unit_tests PRIVATE gtest_main CPLib)
    target_compile_features(unit_tests PUBLIC cxx_std_17)

    include(GoogleTest)
    gtest_discover_tests(unit_tests)
endif()

# =============================
# Usage Instructions
# =============================
# Normal build (no tests):
#   mkdir build && cd build
#   cmake ..
#   cmake --build .
#
# Build with tests:
#   mkdir build && cd build
#   cmake -DBUILD_TESTS=ON ..
#   cmake --build .
#   ctest
