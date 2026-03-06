#!/bin/bash
# Platform Features Test - Linux
# Run from project root: ./tests/run_platform_features_test.sh
# Requires: compiler built (BUILD_DIR env or default "build")

set -e
BUILD_DIR="${BUILD_DIR:-build}"
COMPILER="$BUILD_DIR/Custom-Programming-Language"
OUT_DIR="$BUILD_DIR"
TEST_EXE="$OUT_DIR/platform_features_test"

echo ""
echo "Platform Features Test"
echo "======================"

"$COMPILER" -o platform_features_test --out-dir "$OUT_DIR" tests/inputs/platform_features_test.picoc
printf "hello\n42\n3.14\n" | "$TEST_EXE"
exit $?
