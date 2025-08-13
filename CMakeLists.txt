cmake_minimum_required(VERSION 3.10)  # minimum CMake version
project(Custom-Programming-Language)  # project name

# Search for all .cpp files recursively inside src/
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp)

# Create the executable
add_executable(Custom-Programming-Language src/main.cpp ${SRC_FILES})

# Require C++17
set_target_properties(
    Custom-Programming-Language PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

# Enable warnings
if (MSVC)
    target_compile_options(Custom-Programming-Language PRIVATE /W4 /permissive-)
else()
    target_compile_options(Custom-Programming-Language PRIVATE -Wall -Wextra -pedantic)
endif()

# How to run in terminal:
#mkdir build
#cd build
#cmake ..
#cmake --build .