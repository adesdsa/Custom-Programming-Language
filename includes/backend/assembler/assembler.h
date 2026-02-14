#pragma once

#include <filesystem>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iostream>

#include "includes/shared/common/target_architecture.h"
#include "includes/shared/utils/system_utils.h"
#include "includes/shared/logger/entity_logger.h"

/// @brief Simple wrapper around NASM assembler invocation.
class Assembler
{
public:
    /// @brief Construct an Assembler with a reference to an ErrorManager.
    /// @param errorManager Reference to an error manager for reporting assembly errors.
    Assembler();

    /// @brief Assemble given ASM source code and return path to object file.
    /// @param asmContents The ASM source code to assemble.
    /// @param programName A unique program name used for temporary file generation (without extension).
    /// @return Path to the resulting object file, or an empty path on error.
    std::string Mainflow(const std::string& asmPath, const std::string& programName);

private:
    EntityLogger m_assemblerLogger;

    /// @brief Build the NASM command line for a given ASM file and output object file.
    /// @param nasmPath Path to the NASM executable.
    /// @param asmFile Path to the ASM file to assemble.
    /// @param objFile Path to the resulting object file.
    /// @param nasmFormat NASM output format (e.g., "win64", "elf64").
    /// @return Fully formed command line string for invoking NASM.
    std::string BuildNasmCommand(
        const std::filesystem::path& nasmPath,
        const std::filesystem::path& asmFile,
        const std::filesystem::path& objFile,
        const std::string& nasmFormat) const;

    /// @brief Validate that NASM exists at the given path (Windows only).
    /// @param nasmPath Path to the NASM executable.
    /// @return True if NASM exists and is accessible; false otherwise.
    bool ValidateNasmPath(const std::filesystem::path& nasmPath);
};

