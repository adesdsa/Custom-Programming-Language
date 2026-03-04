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
 *   std::string exePath = linker.MainFlow({"build/main.obj", "build/utils.obj"}, "build/program.exe");
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
    /// @param objFilePaths  Paths to the .obj (Windows) or .o (Linux) files.
    /// @param outputExePath Path to the output executable.
    /// @return Path to the produced executable.
    std::string MainFlow(
        const std::vector<std::string>& objFilePaths,
        const std::string& outputExePath
    );

private:
    EntityLogger m_logger;

    /// @brief Build the platform-specific linker command line.
    /// @param objPaths Paths to the input object files.
    /// @param exePath  Path to the output executable.
    /// @return The full shell command to invoke the linker.
    std::string BuildLinkerCommand(
        const std::vector<std::string>& objPaths,
        const std::string& exePath
    ) const;
};
