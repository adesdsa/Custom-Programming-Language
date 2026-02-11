#pragma once

#include <filesystem>
#include <string>
#include <filesystem>
#include <sstream>
#include <cstdlib>

#include "includes/shared/errors/error_manager.h"
#include "includes/shared/common/target_architecture.h"

class Linker
{
public:
    // Constructor
    explicit Linker(ErrorManager& errorManager);

    /**
     * @brief Link object file into an executable
     * @param objFilePath Path to the object file produced by the assembler
     * @return Path to the linked executable, or empty path on error
     */
    std::filesystem::path Link(const std::filesystem::path& objFilePath);

private:
    ErrorManager& m_errorManager;
};