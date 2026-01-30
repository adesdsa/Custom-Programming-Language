#!/bin/bash
#
# Build script for Custom-Programming-Language
#
# Usage:
#   ./build.sh run          # Build and run the project
#   ./build.sh test         # Build and run tests
#   ./build.sh release      # Build optimized release version
#   ./build.sh clean        # Remove build directory
#   ./build.sh help         # Show available commands

set -e  # Exit on error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
EXE_NAME="Custom-Programming-Language"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
MAGENTA='\033[0;35m'
CYAN='\033[0;36m'
WHITE='\033[1;37m'
GRAY='\033[0;90m'
NC='\033[0m' # No Color

print_logo() {
    echo ""
    echo -e "   ${CYAN}######${NC}  ${YELLOW}####${NC} ${CYAN}#######${NC} ${CYAN}#######${NC}  ${YELLOW}######${NC}"
    echo -e "   ${CYAN}##   ##${NC}  ${YELLOW}##${NC}  ${CYAN}##${NC}      ${CYAN}##   ##${NC} ${YELLOW}##${NC}"
    echo -e "   ${CYAN}######${NC}   ${YELLOW}##${NC}  ${CYAN}##${NC}      ${CYAN}##   ##${NC} ${YELLOW}##${NC}"
    echo -e "   ${CYAN}##${NC}       ${YELLOW}##${NC}  ${CYAN}##${NC}      ${CYAN}##   ##${NC} ${YELLOW}##${NC}"
    echo -e "   ${CYAN}##${NC}      ${YELLOW}####${NC} ${CYAN}#######${NC} ${CYAN}#######${NC}  ${YELLOW}######${NC}"
    echo ""
    echo -e "   ${GRAY}picoC - MIT License${NC}"
    echo ""
}

print_step() {
    echo ""
    echo -e "${YELLOW}  >> $1${NC}"
    echo ""
}

print_separator() {
    echo -e "${GRAY}  --------------------------------------------------------${NC}"
}

build_debug() {
    print_step "Configuring Debug Build"
    cmake --preset debug
    
    print_step "Building Debug"
    cmake --build "$BUILD_DIR" --config Debug
}

build_release() {
    print_logo
    START_TIME=$(date +%s)
    
    print_step "Configuring Release Build"
    cmake --preset release
    
    print_step "Building Release (optimized) - Full Rebuild"
    mkdir -p "$BUILD_DIR"
    cmake --build "$BUILD_DIR" --config Release --clean-first 2>&1 | tee "$BUILD_DIR/release_build.log"
    
    END_TIME=$(date +%s)
    DURATION=$((END_TIME - START_TIME))
    DURATION_STR="${DURATION}s"
    if [ $DURATION -ge 60 ]; then
        MINS=$((DURATION / 60))
        SECS=$((DURATION % 60))
        DURATION_STR="${MINS}m ${SECS}s"
    fi
    
    # Parse build log for statistics
    WARNING_COUNT=$(grep -c -E "warning:|warning C" "$BUILD_DIR/release_build.log" 2>/dev/null || echo "0")
    ERROR_COUNT=$(grep -c -E " error:| error C" "$BUILD_DIR/release_build.log" 2>/dev/null || echo "0")
    
    # Count source files
    CPP_FILES=$(find "$SCRIPT_DIR/src" -name "*.cpp" 2>/dev/null | wc -l | tr -d ' ')
    HEADER_FILES=$(find "$SCRIPT_DIR/includes" -name "*.h" 2>/dev/null | wc -l | tr -d ' ')
    
    # Get executable size
    EXE_PATH="$BUILD_DIR/$EXE_NAME"
    if [[ -f "$EXE_PATH" ]]; then
        EXE_SIZE=$(du -h "$EXE_PATH" 2>/dev/null | cut -f1)
    else
        EXE_SIZE="N/A"
    fi
    
    # Get library size
    LIB_PATH="$BUILD_DIR/libCPLib.a"
    if [[ -f "$LIB_PATH" ]]; then
        LIB_SIZE=$(du -h "$LIB_PATH" 2>/dev/null | cut -f1)
    else
        LIB_SIZE="N/A"
    fi

    # ============================================
    # SHOW WARNINGS FIRST (if any)
    # ============================================
    if [[ "$WARNING_COUNT" -gt 0 ]]; then
        echo ""
        echo -e "${YELLOW}  ╭─────────────────────────────────────────────────────────╮${NC}"
        echo -e "${YELLOW}  │                    COMPILER WARNINGS                    │${NC}"
        echo -e "${YELLOW}  ╰─────────────────────────────────────────────────────────╯${NC}"
        echo ""
        
        grep -E "warning:|warning C" "$BUILD_DIR/release_build.log" | head -15 | while IFS= read -r line; do
            # Shorten the line if too long
            short_line="${line:0:65}"
            if [ ${#line} -gt 65 ]; then
                short_line="${short_line}..."
            fi
            echo -e "  ${GRAY}$short_line${NC}"
        done
        
        if [[ "$WARNING_COUNT" -gt 15 ]]; then
            REMAINING=$((WARNING_COUNT - 15))
            echo ""
            echo -e "  ${YELLOW}... and $REMAINING more warnings${NC}"
        fi
        echo ""
    fi

    # ============================================
    # BUILD STATISTICS
    # ============================================
    echo ""
    echo -e "${CYAN}  ╭─────────────────────────────────────────────────────────╮${NC}"
    echo -e "${CYAN}  │               ${WHITE}BUILD COMPLETE${CYAN}                          │${NC}"
    echo -e "${CYAN}  ╰─────────────────────────────────────────────────────────╯${NC}"
    echo ""
    
    # Status indicator
    if [[ "$ERROR_COUNT" -gt 0 ]]; then
        echo -e "    ${RED}██ FAILED${NC}"
    elif [[ "$WARNING_COUNT" -gt 0 ]]; then
        echo -e "    ${YELLOW}██ SUCCESS with $WARNING_COUNT warning(s)${NC}"
    else
        echo -e "    ${GREEN}██ SUCCESS${NC}"
    fi
    echo ""
    
    # Stats table
    echo -e "${GRAY}  ┌──────────────────┬────────────────────────────────────┐${NC}"
    printf "${GRAY}  │${WHITE} Configuration   ${GRAY}│${CYAN} %-35s${GRAY}│${NC}\n" "Release (optimized)"
    printf "${GRAY}  │${WHITE} Build Time      ${GRAY}│${CYAN} %-35s${GRAY}│${NC}\n" "$DURATION_STR"
    echo -e "${GRAY}  ├──────────────────┼────────────────────────────────────┤${NC}"
    printf "${GRAY}  │${WHITE} Source Files    ${GRAY}│${WHITE} %-35s${GRAY}│${NC}\n" "$CPP_FILES .cpp  |  $HEADER_FILES .h"
    
    if [[ "$WARNING_COUNT" -gt 0 ]]; then
        printf "${GRAY}  │${WHITE} Warnings        ${GRAY}│${YELLOW} %-35s${GRAY}│${NC}\n" "$WARNING_COUNT"
    else
        printf "${GRAY}  │${WHITE} Warnings        ${GRAY}│${GREEN} %-35s${GRAY}│${NC}\n" "0"
    fi
    
    if [[ "$ERROR_COUNT" -gt 0 ]]; then
        printf "${GRAY}  │${WHITE} Errors          ${GRAY}│${RED} %-35s${GRAY}│${NC}\n" "$ERROR_COUNT"
    else
        printf "${GRAY}  │${WHITE} Errors          ${GRAY}│${GREEN} %-35s${GRAY}│${NC}\n" "0"
    fi
    
    echo -e "${GRAY}  ├──────────────────┼────────────────────────────────────┤${NC}"
    printf "${GRAY}  │${WHITE} Executable      ${GRAY}│${MAGENTA} %-35s${GRAY}│${NC}\n" "$EXE_SIZE"
    printf "${GRAY}  │${WHITE} Library         ${GRAY}│${MAGENTA} %-35s${GRAY}│${NC}\n" "$LIB_SIZE"
    echo -e "${GRAY}  └──────────────────┴────────────────────────────────────┘${NC}"
    echo ""
}

COMMAND="${1:-help}"
shift 2>/dev/null || true  # Remove first arg, ignore error if no args

case "$COMMAND" in
    run)
        print_logo
        build_debug
        
        print_step "Running Project"
        EXE_PATH="$BUILD_DIR/$EXE_NAME"
        if [[ -f "$EXE_PATH" ]]; then
            "$EXE_PATH" "$@"
        else
            echo -e "${RED}Executable not found: $EXE_PATH${NC}"
            exit 1
        fi
        ;;
        
    test)
        print_logo
        build_debug
        
        print_step "Running Tests"
        ctest --test-dir "$BUILD_DIR" -C Debug --output-on-failure
        ;;
        
    release)
        build_release
        ;;
        
    debug)
        print_logo
        build_debug
        ;;
        
    clean)
        print_logo
        print_step "Cleaning Build Directory"
        if [[ -d "$BUILD_DIR" ]]; then
            rm -rf "$BUILD_DIR"
            echo -e "  ${GREEN}[OK] Build directory removed.${NC}"
        else
            echo -e "  ${YELLOW}[--] Build directory doesn't exist.${NC}"
        fi
        echo ""
        ;;
        
    help|*)
        print_logo
        echo -e "${WHITE}  USAGE: ./build.sh <command> [args]${NC}"
        echo ""
        print_separator
        echo ""
        echo -e "  ${GREEN}run${NC} [args]     Build debug and run the executable"
        echo "                 Example: ./build.sh run input.txt"
        echo ""
        echo -e "  ${GREEN}test${NC}           Build debug and run all unit tests"
        echo ""
        echo -e "  ${GREEN}release${NC}        Build optimized release version"
        echo ""
        echo -e "  ${GREEN}debug${NC}          Build debug version only"
        echo ""
        echo -e "  ${GREEN}clean${NC}          Remove the build directory"
        echo ""
        echo -e "  ${GREEN}help${NC}           Show this help message"
        echo ""
        print_separator
        echo ""
        ;;
esac
