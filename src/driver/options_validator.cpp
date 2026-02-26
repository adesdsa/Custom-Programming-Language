#include "includes/driver/options_validator.h"

#include "includes/shared/utils/string_utils.hpp"
#include "includes/shared/utils/system_utils.h"

#include <stdexcept>
#include <filesystem>

void OptionsValidator::Validate(const CompilerOptions& options)
{
    // ---- required fields ----

    if (options.inputFilesPath.empty())
    {
        throw std::runtime_error("No input files specified. Use --help for usage.");
    }

    // ---- file extension check ----

    for (const auto& filePath : options.inputFilesPath)
    {
        std::string narrow = StringUtils::ToNarrow(filePath);
        if (!StringUtils::EndsWith(narrow, ".picoc"))
        {
            throw std::runtime_error(
                "Invalid input file: '" + narrow + "'. "
                "Only .picoc files are accepted.");
        }
    }

    // ---- mutual exclusivity ----

    if (options.isVerbose && options.isQuite)
    {
        throw std::runtime_error("Flags '-v' (verbose) and '-q' (quiet) are mutually exclusive.");
    }

    if (options.stopAtAsm && options.stopAtBin)
    {
        throw std::runtime_error("Flags '--asm' and '--bin' are mutually exclusive.");
    }

    // ---- output directory write-permission check (only when user explicitly set one) ----

    if (!options.outputDir.empty())
    {
        std::filesystem::path outputDir(options.outputDir);

        if (outputDir.is_relative())
        {
            outputDir = std::filesystem::current_path() / outputDir;
        }

        if (!SystemUtils::IsDirectoryWritable(outputDir))
        {
            throw std::runtime_error(
                "Permission denied: cannot write to output directory \""
                + outputDir.string()
                + "\". Please run as administrator or choose a different output directory.");
        }
    }
}

