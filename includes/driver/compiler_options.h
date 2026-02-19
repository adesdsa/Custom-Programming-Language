#pragma once

#include <string>
#include <vector>

#define DEFAULT_OUTPUT_PATH L"output"

constexpr int MAX_OPTIMIATION_LEVEL = 1;

struct CompilerOptions
{
    // =======================
    // ====== IO FLAGS =======
    // =======================

    /// @brief path to the input source file (.picoc)
    /// @struct <file_name1>.picoc <file_name2>.picoc ...
    std::vector<std::wstring> inputFilesPath;

    /// @brief desired output executable name (without extension)
    /// @struct -o <output_path>
    std::wstring outputFileName = DEFAULT_OUTPUT_PATH;

    // =======================
    // ==== COMMAND FLAGS ====
    // =======================

    /// print the usage message and exit (--help)
    /// @brief print the usage message and exit
    /// @struct --help
    /// @note should be a stand-alone command 
    bool displayHelp = false;

    /// @brief print picoc version and exist
    /// @struct --version
    /// @note should be a stand-alone command 
    bool displayVersion = false;    

    // =======================
    // ===== LOGS FLAGS ======
    // =======================

    /// @brief Controls which warnings are emitted during compilation:
        //   0 – no warnings (--warn-off)
        //   1 – common warnings (default)
        //   2 – all warnings including pedantic (--warn-all)
    /// @struct 
        // --warn-off
        // --warn-all
    short warningLevel = 0;

    /// @brief print all compiler logs at UI
    /// @struct -v
    bool isVerbose = false;

    /// @brief do not print any logs at UI
    /// @struct -q
    bool isQuite = false;

    /// @brief save logs onto logs files in addition to UI logs
    /// @struct --save-logs
    bool saveLogs = false;

    // ===================================
    // ===== COMPILATION TYPE FLAGS ======
    // ===================================

    /// @brief whether stop compile at linker (stop at .o/.obj files)
    /// @struct --bin
    bool stopAtBin = false;

    /// @brief whether stop compile at assembler (stop at .asm/.s files)
    /// @struct --asm
    bool stopAtAsm = false;

    /// @brief save all temp files created throughout the compilation (.asm, .obj etc)
    /// @struct --save-temp
    bool saveTemps = false;

    // ==============================
    // ===== OPTIMIZATION FLAGS =====
    // ==============================

    /// @brief Controls the aggressiveness of optimizations applied:
        //   0 – no optimizations; fastest compilation
        //   1 – basic optimizations (default)
    /// @struct -O<level>
    short optimizationLevel = MAX_OPTIMIATION_LEVEL;
};
