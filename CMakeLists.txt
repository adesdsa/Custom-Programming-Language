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

# Exclude GUI sources from the shared library (they have their own main & Qt deps)
file(GLOB_RECURSE GUI_SRC_FILES CONFIGURE_DEPENDS src/driver/ui/gui/*.cpp)
if(GUI_SRC_FILES)
    list(REMOVE_ITEM SRC_FILES ${GUI_SRC_FILES})
endif()

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

# =============================
# GUI (Qt)
# =============================
# Requires Qt 6 installed separately (https://www.qt.io/download-qt-installer).
# Pass -DCMAKE_PREFIX_PATH=<Qt-install-dir> so CMake can find it.
# Example: cmake .. -DBUILD_GUI=ON -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc2022_64"
option(BUILD_GUI "Build picoc-gui (requires Qt 6)" OFF)
if(BUILD_GUI)
    set(CMAKE_AUTOMOC ON)
    find_package(Qt6 REQUIRED COMPONENTS Widgets)

    file(GLOB_RECURSE GUI_SOURCES CONFIGURE_DEPENDS src/driver/ui/gui/*.cpp)
    file(GLOB_RECURSE GUI_HEADERS CONFIGURE_DEPENDS includes/driver/ui/gui/*.h)
    add_executable(picoc-gui ${GUI_SOURCES} ${GUI_HEADERS})
    target_include_directories(picoc-gui PRIVATE ${CMAKE_SOURCE_DIR})
    target_link_libraries(picoc-gui PRIVATE CPLib Qt6::Widgets)
    set_target_properties(picoc-gui PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
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