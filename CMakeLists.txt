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
    # Pick 32-bit or 64-bit NASM depending on target architecture
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(NASM_EXECUTABLE "${CMAKE_SOURCE_DIR}/tools/nasm/nasm64.exe")
    else()
        set(NASM_EXECUTABLE "${CMAKE_SOURCE_DIR}/tools/nasm/nasm32.exe")
    endif()

    if(NOT EXISTS ${NASM_EXECUTABLE})
        message(FATAL_ERROR "NASM executable not found at ${NASM_EXECUTABLE}. Place it in tools/nasm/")
    endif()
    message(STATUS "Using NASM: ${NASM_EXECUTABLE}")

else() # Linux / macOS
    find_program(NASM_EXECUTABLE nasm)
    if(NOT NASM_EXECUTABLE)
        message(STATUS "NASM not found, attempting to install via apt...")
        execute_process(COMMAND sudo apt-get update)
        execute_process(COMMAND sudo apt-get install -y nasm
                        RESULT_VARIABLE NASM_RESULT
                        OUTPUT_QUIET ERROR_QUIET)
        if(NASM_RESULT EQUAL 0)
            find_program(NASM_EXECUTABLE nasm)
            if(NOT NASM_EXECUTABLE)
                message(FATAL_ERROR "NASM installation failed. Please install manually.")
            else()
                message(STATUS "NASM installed successfully: ${NASM_EXECUTABLE}")
            endif()
        else()
            message(FATAL_ERROR "Failed to auto-install NASM. Please install manually.")
        endif()
    else()
        message(STATUS "NASM found: ${NASM_EXECUTABLE}")
    endif()
endif()


# ==============================
# Linker
# ==============================
if(WIN32)
    # Try to find link.exe in PATH first
    find_program(LINK_EXECUTABLE link)
    
    # If not found, try to detect Visual Studio automatically
    if(NOT LINK_EXECUTABLE AND MSVC)
        if(DEFINED ENV{VSINSTALLDIR})
            set(LINK_EXECUTABLE "$ENV{VSINSTALLDIR}/VC/Tools/MSVC/${CMAKE_VS_MSVC_VERSION}/bin/Hostx64/x64/link.exe")
        endif()
    endif()

    if(NOT LINK_EXECUTABLE OR NOT EXISTS ${LINK_EXECUTABLE})
        message(FATAL_ERROR "link.exe not found. Install Visual Studio Build Tools and ensure PATH or VSINSTALLDIR is correct.")
    endif()

    message(STATUS "Using link.exe: ${LINK_EXECUTABLE}")

else() # Linux / macOS
    find_program(LD_EXECUTABLE ld)
    if(NOT LD_EXECUTABLE)
        if(UNIX)
            message(STATUS "ld not found, attempting to install binutils...")
            execute_process(COMMAND sudo apt-get update)
            execute_process(COMMAND sudo apt-get install -y binutils
                            RESULT_VARIABLE LD_RESULT
                            OUTPUT_QUIET ERROR_QUIET)
            if(LD_RESULT EQUAL 0)
                find_program(LD_EXECUTABLE ld)
                if(NOT LD_EXECUTABLE)
                    message(FATAL_ERROR "ld installation failed. Please install manually.")
                else()
                    message(STATUS "ld installed successfully: ${LD_EXECUTABLE}")
                endif()
            else()
                message(FATAL_ERROR "Failed to auto-install ld. Please install manually.")
            endif()
        else()
            message(FATAL_ERROR "ld not found. Please install binutils.")
        endif()
    else()
        message(STATUS "Using ld: ${LD_EXECUTABLE}")
    endif()
endif()
