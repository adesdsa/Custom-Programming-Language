cmake_minimum_required(VERSION 3.14)

project(Custom-Programming-Language LANGUAGES CXX)

# -----------------------------
# MSVC Runtime + GoogleTest config
# MUST come BEFORE FetchContent
# -----------------------------
if(MSVC)
    # /MDd for Debug, /MD for Release
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" CACHE STRING "" FORCE)

    # Force googletest to use shared CRT (/MD, /MDd)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(BUILD_GTEST ON CACHE BOOL "" FORCE)
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

    # Prevent any system-installed GTest from being detected
    set(GTest_ROOT "" CACHE PATH "" FORCE)
endif()

# =============================
# FetchContent
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
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG main
)
FetchContent_MakeAvailable(googletest)

# =============================
# Source Files
# =============================
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp)
list(REMOVE_ITEM SRC_FILES "${CMAKE_SOURCE_DIR}/main.cpp")

# =============================
# CPLib
# =============================
add_library(CPLib ${SRC_FILES})
target_include_directories(CPLib PUBLIC ${CMAKE_SOURCE_DIR}/includes)

target_link_libraries(CPLib PUBLIC mio gtest)

set_target_properties(CPLib PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

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

set_target_properties(Custom-Programming-Language PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# =============================
# Tests (optional)
# =============================
option(BUILD_TESTS "Build unit tests" OFF)

if(BUILD_TESTS)
    enable_testing()
    file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS tests/*.cpp)

    add_executable(unit_tests ${TEST_SOURCES})
    target_link_libraries(unit_tests PRIVATE gtest_main CPLib)

    set_target_properties(unit_tests PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )

    include(GoogleTest)
    gtest_discover_tests(unit_tests)
endif()

# =============================
# Usage 
# =============================
# Normal build (no tests): 
# mkdir build
# cmake -S . -B build
# cmake --build build --config Debug
# 
# Build with tests: 
# mkdir build
# cmake -S . -B build -DBUILD_TESTS=ON
# cmake --build build --config Debug
# ctest
