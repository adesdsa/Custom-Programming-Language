#pragma once

#include "includes/driver/compiler_options.h"

class CLIParser
{
public:
    /// @brief Parse command-line arguments into a CompilerOptions struct.
    /// @param argc argument count from main
    /// @param argv argument vector from main
    /// @return populated CompilerOptions
    /// @throws std::runtime_error on invalid arguments
    static CompilerOptions Parse(int argc, const char* argv[]);

    /// @brief Print usage/help message to stdout.
    static void PrintUsage();

    /// @brief Print compiler version to stdout.
    static void PrintVersion();

private:
    CLIParser() = delete;
};
