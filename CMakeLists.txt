cmake_minimum_required(VERSION 3.14)

project(Custom-Programming-Language LANGUAGES CXX)

# -----------------------------
# MSVC Runtime config
# -----------------------------
if(MSVC)
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

# =============================
# Bundled Toolchain Paths
# =============================
if(WIN32)
    add_compile_definitions(GOLINK_TOOL_PATH="${CMAKE_SOURCE_DIR}/tools/golink/GoLink.exe")
endif()

target_link_libraries(CPLib PUBLIC mio::mio GTest::gtest)

set_target_properties(CPLib PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# =============================
# Compiler warnings
# =============================
if(MSVC)
    target_compile_options(CPLib PRIVATE /W4 /permissive-)
else()
    target_compile_options(CPLib PRIVATE -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion)
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

# ==============================
# NASM
# ==============================
if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(NASM_EXECUTABLE "${CMAKE_SOURCE_DIR}/tools/nasm/nasm64.exe")
    else()
        set(NASM_EXECUTABLE "${CMAKE_SOURCE_DIR}/tools/nasm/nasm32.exe")
    endif()

    if(NOT EXISTS ${NASM_EXECUTABLE})
        message(FATAL_ERROR "NASM executable not found at ${NASM_EXECUTABLE}. Place it in tools/nasm/")
    endif()
    message(STATUS "Using NASM: ${NASM_EXECUTABLE}")

else() # Linux/macOS
    find_program(NASM_EXECUTABLE nasm)

    if(NOT NASM_EXECUTABLE)
        # Try automatic install if apt-get exists
        find_program(APT_EXECUTABLE apt-get)
        if(APT_EXECUTABLE)
            message(STATUS "NASM not found. Attempting to install via apt-get...")
            execute_process(COMMAND sudo apt-get update RESULT_VARIABLE _update_result OUTPUT_QUIET ERROR_QUIET)
            execute_process(COMMAND sudo apt-get install -y nasm RESULT_VARIABLE _install_result OUTPUT_QUIET ERROR_QUIET)
            if(_install_result EQUAL 0)
                find_program(NASM_EXECUTABLE nasm)
            endif()
        endif()

        if(NOT NASM_EXECUTABLE)
            message(FATAL_ERROR "NASM not found. On Debian/Ubuntu install with: sudo apt install nasm")
        endif()
    endif()
    message(STATUS "Using NASM: ${NASM_EXECUTABLE}")
endif()