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
        # Only try apt-get if it exists
        find_program(APT_GET_EXECUTABLE apt-get)
        if(APT_GET_EXECUTABLE)
            message(STATUS "NASM not found, attempting to install via apt-get...")
            execute_process(COMMAND sudo apt-get update
                            RESULT_VARIABLE _apt_update_result
                            OUTPUT_QUIET ERROR_QUIET)
            execute_process(COMMAND sudo apt-get install -y nasm
                            RESULT_VARIABLE NASM_RESULT
                            OUTPUT_QUIET ERROR_QUIET)
            if(NASM_RESULT EQUAL 0)
                find_program(NASM_EXECUTABLE nasm)
            endif()
        endif()

        if(NOT NASM_EXECUTABLE)
            message(FATAL_ERROR "NASM not found. On Debian/Ubuntu you can install with: sudo apt install nasm")
        else()
            message(STATUS "NASM installed/found: ${NASM_EXECUTABLE}")
        endif()
    else()
        message(STATUS "NASM found: ${NASM_EXECUTABLE}")
    endif()
endif()


# ==============================
# Linker: link.exe on Windows, ld on Linux
# ==============================
if(WIN32)
    # 1) Try PATH
    find_program(LINK_EXECUTABLE link)

    # 2) If not found, try vswhere to locate Visual Studio installation and search for link.exe
    if(NOT LINK_EXECUTABLE)
        set(VSWHERE_PATH "C:/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe")
        if(EXISTS ${VSWHERE_PATH})
            execute_process(
                COMMAND "${VSWHERE_PATH}" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
                OUTPUT_VARIABLE VS_INSTALL_PATH
                ERROR_QUIET
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )
            if(VS_INSTALL_PATH)
                # search for link.exe under installation path (Tools/MSVC versions)
                file(GLOB_RECURSE _link_candidates RELATIVE "${VS_INSTALL_PATH}" "${VS_INSTALL_PATH}/**/link.exe")
                if(_link_candidates)
                    # pick the first candidate and construct absolute path
                    list(GET _link_candidates 0 _first_relative)
                    set(LINK_EXECUTABLE "${VS_INSTALL_PATH}/${_first_relative}")
                endif()
            endif()
        endif()
    endif()

    # 3) As a last resort, try common VS paths using CMAKE_VS_MSVC_VERSION if available
    if(NOT LINK_EXECUTABLE AND DEFINED CMAKE_VS_MSVC_VERSION)
        foreach(_hostarch Hostx64 Hostx86)
            foreach(_targetarch x64 x86)
                set(_possible "${ProgramFiles}/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/${CMAKE_VS_MSVC_VERSION}/bin/${_hostarch}/${_targetarch}/link.exe")
                if(EXISTS "${_possible}")
                    set(LINK_EXECUTABLE "${_possible}")
                endif()
            endforeach()
        endforeach()
    endif()

    if(NOT LINK_EXECUTABLE OR NOT EXISTS ${LINK_EXECUTABLE})
        message(FATAL_ERROR "link.exe not found. Please install Visual Studio Build Tools (C++ workload). "
            "Recommended: open 'x64 Native Tools Command Prompt for VS' or run vcvarsall.bat before configuring CMake.\n"
            "If Visual Studio is installed, make sure vswhere.exe is present under 'C:/Program Files (x86)/Microsoft Visual Studio/Installer/'."
        )
    endif()

    message(STATUS "Using Windows linker: ${LINK_EXECUTABLE}")

else() # UNIX (Linux / macOS)
    find_program(LD_EXECUTABLE ld)
    if(NOT LD_EXECUTABLE)
        # Only try apt-get if it exists
        find_program(APT_GET_EXECUTABLE apt-get)
        if(APT_GET_EXECUTABLE)
            message(STATUS "ld not found, attempting to install binutils via apt-get...")
            execute_process(COMMAND sudo apt-get update
                            RESULT_VARIABLE _apt_update_result
                            OUTPUT_QUIET ERROR_QUIET)
            execute_process(COMMAND sudo apt-get install -y binutils
                            RESULT_VARIABLE LD_RESULT
                            OUTPUT_QUIET ERROR_QUIET)
            if(LD_RESULT EQUAL 0)
                find_program(LD_EXECUTABLE ld)
            endif()
        endif()
    endif()

    if(NOT LD_EXECUTABLE)
        message(FATAL_ERROR "ld (binutils) not found. On Debian/Ubuntu install with: sudo apt install binutils")
    endif()

    message(STATUS "Using ld: ${LD_EXECUTABLE}")
endif()
