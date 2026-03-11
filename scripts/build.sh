#!/bin/bash
# picoC multi-platform build script (Linux)
# Usage: ./scripts/build.sh [command] [args]
#   run      - Build and run (linux64)
#   test     - Build and run all tests (linux64)
#   test32   - Build and run tests for linux32
#   release  - Build release

set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$PROJECT_ROOT"

COMMAND="${1:-help}"

logo() {
    echo
    echo "   ######  #### ####### #######  ###### "
    echo "   ##   ##  ##  ##      ##   ## ##      "
    echo "   ######   ##  ##      ##   ## ##      "
    echo "   ##       ##  ##      ##   ## ##      "
    echo "   ##      #### ####### #######  ###### "
    echo
    echo "   picoC - MIT License"
    echo
}

case "$COMMAND" in
    run)
        logo
        echo "   >> Configuring Debug Build (linux64)"
        cmake --preset linux64 2>/dev/null || cmake --preset debug 2>/dev/null || {
            mkdir -p build && cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=OFF \
                ${VCPKG_ROOT:+-DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake}
        }
        echo "   >> Building"
        cmake --build build
        echo "   >> Running"
        ./build/Custom-Programming-Language "${@:2}"
        ;;
    test)
        logo
        echo "   >> Configuring Debug Build (linux64)"
        cmake --preset linux64 2>/dev/null || cmake --preset debug 2>/dev/null || {
            mkdir -p build && cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON \
                ${VCPKG_ROOT:+-DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake}
        }
        echo "   >> Building"
        cmake --build build
        echo "   >> Running Tests"
        ctest --test-dir build --output-on-failure
        ;;
    test32)
        logo
        echo "   >> Configuring Debug Build (linux32)"
        echo "   >> Requires: g++-multilib, gcc-multilib, libc6-dev-i386"
        cmake --preset linux32 2>/dev/null || {
            mkdir -p build-linux32
            cmake -S . -B build-linux32 -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON \
                -DCMAKE_C_FLAGS="-m32" -DCMAKE_CXX_FLAGS="-m32" \
                -DCMAKE_EXE_LINKER_FLAGS="-m32" -DCMAKE_SHARED_LINKER_FLAGS="-m32" \
                ${VCPKG_ROOT:+-DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake}
        }
        echo "   >> Building"
        cmake --build build-linux32
        echo "   >> Running Tests"
        ctest --test-dir build-linux32 --output-on-failure
        ;;
    release)
        logo
        echo "   >> Configuring Release Build"
        cmake --preset release 2>/dev/null || {
            mkdir -p build && cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
                ${VCPKG_ROOT:+-DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake}
        }
        echo "   >> Building"
        cmake --build build --config Release
        echo "   >> Done"
        ;;
    help|*)
        logo
        echo "   USAGE: ./scripts/build.sh <command> [args]"
        echo
        echo "   run [args]   Build debug and run the executable"
        echo "   test         Build with tests and run all unit tests (linux64)"
        echo "   test32       Build and run tests for linux32 (requires multilib)"
        echo "   release      Build optimized release version"
        echo "   help         Show this help"
        echo
        ;;
esac
