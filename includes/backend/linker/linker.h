#pragma once

#include <string>
#include <vector>

#include "includes/shared/logger/entity_logger.h"
#include "includes/shared/common/target_architecture.h"
#include "includes/shared/utils/system_utils.h"

#include <filesystem>
#include <stdexcept>

constexpr int OK_EXIT_CODE = 0;

/**
 * @brief Links object files (.obj / .o) into a native executable.
 *
 * On Windows the bundled GoLink.exe is used (no SDK / .lib files required).
 * On Linux the system linker `ld` is invoked.
 * 
 * @note Most of the linker is handled compile time (via constexpr).
 *
 * Usage:
 *   Linker linker;
 *   std::string exePath = linker.MainFlow({"build/main.obj", "build/utils.obj"}, "program", "build/");
 * 
 * References:
 *  GoLink:
 *      @link - https://www.godevtool.com/GolinkHelp/GoLink.htm
 *  ld:
 *      @link - https://linux.die.net/man/1/ld
 */
class Linker
{
public:
    Linker();
    ~Linker() = default;

    /// @brief Links the given object files into a native executable.
    /// @param objFilePaths Paths to the .obj (Windows) or .o (Linux) files.
    /// @param outputName   Base name of the executable (without extension).
    /// @param outputDir    Directory where the executable is placed.
    /// @return Path to the produced executable.
    std::string MainFlow(
        const std::vector<std::string>& objFilePaths,
        const std::string& outputName,
        const std::string& outputDir
    );

private:
    EntityLogger m_logger;

    /// @brief Build the platform-specific linker command line.
    std::string BuildLinkerCommand(
        const std::vector<std::string>& objPaths,
        const std::string& exePath
    ) const;

    /// @brief Derive the full executable path from a base name and directory.
    std::string DeriveOutputPath(
        const std::string& outputName,
        const std::string& outputDir
    ) const;
};
