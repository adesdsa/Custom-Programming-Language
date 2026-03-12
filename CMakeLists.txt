cmake_minimum_required(VERSION 3.14)

project(Custom-Programming-Language LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------
# MSVC Runtime config
# -----------------------------
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" CACHE STRING "" FORCE)
    add_compile_options(/FS)  # Allow multiple CL.EXE to write to same PDB (parallel builds)
endif()

# =============================
# Find Packages (installed by vcpkg when available)
# =============================

# Try to find mio via a package manager (e.g., vcpkg). If unavailable, fall back
# to fetching the header-only library directly so the project remains buildable
# on systems without vcpkg.
find_package(mio CONFIG QUIET)

if(NOT mio_FOUND)
    message(STATUS "mio package not found via CMake; fetching header-only mio from GitHub...")
    include(FetchContent)
    FetchContent_Declare(
        mio_external
        GIT_REPOSITORY https://github.com/mandreyel/mio.git
        GIT_TAG 8b6b7d878c89e81614d05edca7936de41ccdd2da
    )
    FetchContent_MakeAvailable(mio_external)

    # Ensure a mio::mio target exists, compatible with the vcpkg one.
    if(NOT TARGET mio::mio)
        if(TARGET mio)
            add_library(mio::mio ALIAS mio)
        else()
            add_library(mio INTERFACE)
            target_include_directories(mio INTERFACE ${mio_external_SOURCE_DIR}/include)
            add_library(mio::mio ALIAS mio)
        endif()
    endif()
endif()

# Try to find GTest via a package manager; if unavailable, fetch googletest.
find_package(GTest CONFIG QUIET)

if(NOT GTest_FOUND)
    message(STATUS "GTest package not found via CMake; fetching googletest from GitHub...")
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    # Recommended by googletest for projects that use shared CRT on Windows; harmless elsewhere.
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

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

target_link_libraries(CPLib PUBLIC mio::mio GTest::gtest)

set_target_properties(CPLib PROPERTIES CXX_EXTENSIONS OFF)

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
target_link_libraries(Custom-Programming-Language PRIVATE CPLib)

# =============================
# Tests
# =============================
option(BUILD_TESTS "Build unit tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS tests/test_scripts/*.cpp)
    add_executable(unit_tests ${TEST_SOURCES})
    target_link_libraries(unit_tests PRIVATE GTest::gtest_main CPLib)
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
    set(CMAKE_AUTORCC ON)
    find_package(Qt6 REQUIRED COMPONENTS Widgets SvgWidgets)

    file(GLOB_RECURSE GUI_SOURCES CONFIGURE_DEPENDS src/driver/ui/gui/*.cpp)
    file(GLOB_RECURSE GUI_HEADERS CONFIGURE_DEPENDS includes/driver/ui/gui/*.h)

    set(GUI_RESOURCES "${CMAKE_SOURCE_DIR}/resources/gui_resources.qrc")

    set(GUI_RC "")
    if(WIN32)
        set(GUI_RC "${CMAKE_SOURCE_DIR}/resources/app.rc")
    endif()

    add_executable(picoc-gui ${GUI_SOURCES} ${GUI_HEADERS} ${GUI_RESOURCES} ${GUI_RC})
    target_include_directories(picoc-gui PRIVATE ${CMAKE_SOURCE_DIR})
    target_link_libraries(picoc-gui PRIVATE CPLib Qt6::Widgets Qt6::SvgWidgets)
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

# ==============================
# picoC stdlib (string, arraylib) - required for linking programs that use strings/arrays
# ==============================
set(STDLIB_OUT_DIR "${CMAKE_SOURCE_DIR}/lib")
set(STDLIB_SRC_DIR "${CMAKE_SOURCE_DIR}/lib/stdlib")

if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(NASM_FMT "win64")
        set(STDLIB_STRING_ASM string_win64.asm)
        set(STDLIB_STRING_OBJ string_win64.obj)
        set(STDLIB_ARRAYLIB_ASM arraylib_win64.asm)
        set(STDLIB_ARRAYLIB_OBJ arraylib_win64.obj)
    else()
        set(NASM_FMT "win32")
        set(STDLIB_STRING_ASM string_win32.asm)
        set(STDLIB_STRING_OBJ string_win32.obj)
        set(STDLIB_ARRAYLIB_ASM arraylib_win32.asm)
        set(STDLIB_ARRAYLIB_OBJ arraylib_win32.obj)
    endif()
else()
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(NASM_FMT "elf64")
        set(STDLIB_STRING_ASM string_linux64.asm)
        set(STDLIB_STRING_OBJ string_linux64.o)
        set(STDLIB_ARRAYLIB_ASM arraylib_linux64.asm)
        set(STDLIB_ARRAYLIB_OBJ arraylib_linux64.o)
    else()
        set(NASM_FMT "elf32")
        set(STDLIB_STRING_ASM string_linux32.asm)
        set(STDLIB_STRING_OBJ string_linux32.o)
        set(STDLIB_ARRAYLIB_ASM arraylib_linux32.asm)
        set(STDLIB_ARRAYLIB_OBJ arraylib_linux32.o)
    endif()
endif()

add_custom_command(
    OUTPUT "${STDLIB_OUT_DIR}/${STDLIB_STRING_OBJ}"
    COMMAND ${NASM_EXECUTABLE} -f ${NASM_FMT} "${STDLIB_SRC_DIR}/${STDLIB_STRING_ASM}" -o "${STDLIB_OUT_DIR}/${STDLIB_STRING_OBJ}"
    DEPENDS "${STDLIB_SRC_DIR}/${STDLIB_STRING_ASM}"
    COMMENT "Assembling stdlib: ${STDLIB_STRING_ASM} -> ${STDLIB_STRING_OBJ}"
)
add_custom_command(
    OUTPUT "${STDLIB_OUT_DIR}/${STDLIB_ARRAYLIB_OBJ}"
    COMMAND ${NASM_EXECUTABLE} -f ${NASM_FMT} "${STDLIB_SRC_DIR}/${STDLIB_ARRAYLIB_ASM}" -o "${STDLIB_OUT_DIR}/${STDLIB_ARRAYLIB_OBJ}"
    DEPENDS "${STDLIB_SRC_DIR}/${STDLIB_ARRAYLIB_ASM}"
    COMMENT "Assembling stdlib: ${STDLIB_ARRAYLIB_ASM} -> ${STDLIB_ARRAYLIB_OBJ}"
)

add_custom_target(picoc_stdlib ALL
    DEPENDS "${STDLIB_OUT_DIR}/${STDLIB_STRING_OBJ}" "${STDLIB_OUT_DIR}/${STDLIB_ARRAYLIB_OBJ}"
)
