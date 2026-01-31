cmake_minimum_required(VERSION 3.14)

project(Custom-Programming-Language LANGUAGES CXX)

# -----------------------------
# MSVC Runtime config
# -----------------------------
if(MSVC)
    # /MDd for Debug, /MD for Release
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" CACHE STRING "" FORCE)
endif()

# =============================
# Find Packages (installed by vcpkg)
# =============================
find_package(mio CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

# =============================
# Source Files
# =============================
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp)
list(REMOVE_ITEM SRC_FILES "${CMAKE_SOURCE_DIR}/main.cpp")

# =============================
# CPLib
# =============================
add_library(CPLib ${SRC_FILES})
target_include_directories(CPLib PUBLIC ${CMAKE_SOURCE_DIR})
add_compile_definitions(PROJECT_ROOT="${CMAKE_SOURCE_DIR}")

target_link_libraries(CPLib PUBLIC mio::mio GTest::gtest)

set_target_properties(CPLib PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Compiler warnings (comprehensive)
if(MSVC)
    target_compile_options(CPLib PRIVATE
        /W4             # High warning level
        /permissive-    # Strict standards conformance
        /w14242         # Conversion warnings
        /w14254         # Operator conversion warnings  
        /w14263         # Member function hiding
        /w14265         # Class has virtual functions but destructor is not virtual
        /w14287         # Unsigned/negative constant mismatch
        /w14296         # Expression is always true/false
        /w14311         # Pointer truncation
        /w14545         # Expression before comma evaluates to a function
        /w14546         # Function call before comma missing argument list
        /w14547         # Operator before comma has no effect
        /w14549         # Operator before comma has no effect
        /w14555         # Expression has no effect
        /w14619         # Pragma warning invalid
        /w14640         # Thread unsafe static member initialization
        /w14826         # Conversion is sign-extended
        /w14905         # Wide string literal cast to LPSTR
        /w14906         # String literal cast to LPWSTR
        /w14928         # Illegal copy-initialization
    )
else()
    target_compile_options(CPLib PRIVATE
        -Wall           # All common warnings
        -Wextra         # Extra warnings not in -Wall
        -Wpedantic      # Strict ISO C++ compliance
        -Wshadow        # Variable shadowing
        -Wconversion    # Implicit type conversions
        -Wsign-conversion # Sign conversion warnings
        -Wnull-dereference # Null pointer dereference
        -Wdouble-promotion # Float implicitly promoted to double
        -Wformat=2      # Printf/scanf format string issues
        -Wcast-align    # Pointer cast increases alignment
        -Wunused        # Unused variables/functions
        -Wnon-virtual-dtor # Non-virtual destructor in class with virtual functions
        -Woverloaded-virtual # Overloaded virtual function
        -Wold-style-cast # C-style casts
    )
endif()

# =============================
# Main executable
# =============================
add_executable(Custom-Programming-Language main.cpp)
target_link_libraries(Custom-Programming-Language PRIVATE CPLib mio::mio)

set_target_properties(Custom-Programming-Language PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# =============================
# Tests
# =============================
option(BUILD_TESTS "Build unit tests" OFF)

if(BUILD_TESTS)
    enable_testing()
    file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS tests/test_scripts/*.cpp)

    add_executable(unit_tests ${TEST_SOURCES})
    target_link_libraries(unit_tests PRIVATE GTest::gtest_main CPLib)

    set_target_properties(unit_tests PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )

    include(GoogleTest)
    gtest_discover_tests(unit_tests)
endif()
