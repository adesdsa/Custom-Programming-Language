#pragma once

#include <string>

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
 *   std::string exePath = linker.MainFlow("build/program.obj");
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

    /// @brief Links the given object file into a native executable.
    /// @param objFilePath  Path to the .obj (Windows) or .o (Linux) file.
    /// @return Path to the produced executable.
    std::string MainFlow(const std::string& objFilePath);

private:
    EntityLogger m_logger;

    /// @brief Build the platform-specific linker command line.
    /// @param objPath Path to the input object file.
    /// @param exePath Path to the output executable.
    /// @return The full shell command to invoke the linker.
    std::string BuildLinkerCommand(
        const std::string& objPath,
        const std::string& exePath
    ) const;

    /// @brief Derive the output executable path from the object file path.
    /// @param objPath Path to the input object file.
    /// @return Path with the correct executable extension (.exe / none).
    std::string DeriveOutputPath(const std::string& objPath) const;
};
