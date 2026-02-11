#pragma once

#include <fstream>
#include <filesystem>
#include <sstream>
#include <stdexcept>
#include <cstdlib>
#include <chrono>

#include "includes/shared/common/target_architecture.h"
#include "includes/shared/errors/error_manager.h"

class Assembler
{
public:
    Assembler(ErrorManager& errorManager);

    std::filesystem::path Assemble(const std::string& asmContents);

private:
    ErrorManager& m_errorManager;
};
