cmake_minimum_required(VERSION 3.10)  # minimum CMake version
project(Custom-Programming-Language)  # project name

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp)

list(REMOVE_ITEM SRC_FILES "${CMAKE_SOURCE_DIR}/src/main.cpp")  # exclude main.cpp

add_executable(Custom-Programming-Language main.cpp ${SRC_FILES})

# Require C++17
set_target_properties(
    Custom-Programming-Language PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

# Add includes/ directory for headers
target_include_directories(Custom-Programming-Language PRIVATE
    ${CMAKE_SOURCE_DIR}/includes
)

# Enable warnings
if (MSVC)
    target_compile_options(Custom-Programming-Language PRIVATE /W4 /permissive-)
else()
    target_compile_options(Custom-Programming-Language PRIVATE -Wall -Wextra -pedantic)
endif()

# How to run in terminal:
#cd build
#cmake ..
#cmake --build .