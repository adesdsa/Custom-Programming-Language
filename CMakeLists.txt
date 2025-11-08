cmake_minimum_required(VERSION 3.14)
project(Custom-Programming-Language)

# =============================
# FetchContent for dependencies
# =============================
include(FetchContent)

# ----- Mio -----
FetchContent_Declare(
    mio
    GIT_REPOSITORY https://github.com/mandreyel/mio.git
    GIT_TAG master
)
FetchContent_MakeAvailable(mio)

# ----- GoogleTest -----
# Fetch GoogleTest regardless of BUILD_TESTS
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG main
)
FetchContent_MakeAvailable(googletest)

# =============================
# Source Files - src/
# =============================
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp)
list(REMOVE_ITEM SRC_FILES "${CMAKE_SOURCE_DIR}/src/main.cpp")

# =============================
# Headers - includes/
# =============================
add_library(CPLib ${SRC_FILES})
target_include_directories(CPLib PUBLIC ${CMAKE_SOURCE_DIR}/includes)
target_link_libraries(CPLib PUBLIC mio gtest)  # GTest added here

# =============================
# Settings
# =============================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
target_compile_features(CPLib PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(CPLib PRIVATE /W4 /permissive-)
else()
    target_compile_options(CPLib PRIVATE -Wall -Wextra -pedantic)
endif()

# =============================
# Main executable
# =============================
add_executable(Custom-Programming-Language main.cpp)
target_link_libraries(Custom-Programming-Language PRIVATE CPLib mio)
target_compile_features(Custom-Programming-Language PUBLIC cxx_std_17)

# =============================
# Tests (optional)
# =============================
option(BUILD_TESTS "Build unit tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS tests/*.cpp)
    add_executable(unit_tests ${TEST_SOURCES})
    target_link_libraries(unit_tests PRIVATE gtest_main CPLib)
    target_compile_features(unit_tests PUBLIC cxx_std_17)
    include(GoogleTest)
    gtest_discover_tests(unit_tests)
endif()

# ============================= 
# Usage 
# ============================= 
# Normal build (no tests): 
# cd build 
# cmake .. 
# cmake --build . 
# 
# Build with tests: 
# cd build 
# cmake -DBUILD_TESTS=ON .. 
# cmake --build . 
# ctest