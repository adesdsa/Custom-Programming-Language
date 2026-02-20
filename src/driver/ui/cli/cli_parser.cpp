#include "includes/driver/ui/cli/cli_parser.h"
#include "includes/driver/options_validator.h"

#include "includes/shared/utils/string_utils.hpp"

#include <iostream>
#include <string>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

/// Hold boolean flags: flag string -> pointer-to-member
static const std::unordered_map<std::string, bool CompilerOptions::*> BOOL_FLAGS = {
    {"--help",      &CompilerOptions::displayHelp},
    {"--version",   &CompilerOptions::displayVersion},
    {"-v",          &CompilerOptions::isVerbose},
    {"-q",          &CompilerOptions::isQuite},
    {"--save-logs", &CompilerOptions::saveLogs},
    {"--bin",       &CompilerOptions::stopAtBin},
    {"--asm",       &CompilerOptions::stopAtAsm},
    {"--save-temp", &CompilerOptions::saveTemps},
};

/// Warning level flags: flag string -> enum value
static const std::unordered_map<std::string, WarningLevel> WARNING_FLAGS = {
    {"--warn-off", WarningLevel::OFF},
    {"--warn-all", WarningLevel::ALL},
};

CompilerOptions CLIParser::Parse(int argc, const char* argv[])
{
    CompilerOptions options;
    std::unordered_set<std::string> seenFlags;

    // Helper: record a flag key and throw if it was already seen.
    // For flags that share the same option slot (e.g. --warn-off / --warn-all)
    // use a common canonical key so they conflict with each other.
    auto requireUnique = [&seenFlags](const std::string& canonicalKey,
                                      const std::string& displayFlag)
    {
        if (!seenFlags.insert(canonicalKey).second)
        {
            throw std::runtime_error(
                "Duplicate flag: '" + displayFlag + "' was already specified.");
        }
    };

    for (int i = 1; i < argc; i++)
    {
        std::string arg(argv[i]);

        // ---- boolean flags ----
        auto flagIterator = BOOL_FLAGS.find(arg);
        if (flagIterator != BOOL_FLAGS.end())
        {
            requireUnique(arg, arg);
            // turn on the flag
            options.*(flagIterator->second) = true;
        }

        // ---- warning level flags ----
        else if (auto warnIt = WARNING_FLAGS.find(arg); warnIt != WARNING_FLAGS.end())
        {
            // --warn-off and --warn-all set the same option; use a common key
            requireUnique("--warn-level", arg);
            options.warningLevel = warnIt->second;
        }

        // ---- output flag ----
        else if (arg == "-o")
        {
            requireUnique("-o", "-o");

            if (i + 1 >= argc)
            {
                throw std::runtime_error("Flag '-o' requires an output file name.");
            }

            std::string nextArg(argv[i + 1]);
            if (StringUtils::StartsWith(nextArg, "-"))
            {
                throw std::runtime_error(
                    "Flag '-o' requires an output file name, but got flag '" + nextArg + "'.");
            }

            options.outputFileName = StringUtils::ToWide(argv[++i]);
        }

        // ---- output directory flag ----
        else if (arg == "--out-dir")
        {
            requireUnique("--out-dir", "--out-dir");

            if (i + 1 >= argc)
            {
                throw std::runtime_error("Flag '--out-dir' requires a directory path.");
            }

            std::string nextArg(argv[i + 1]);
            if (StringUtils::StartsWith(nextArg, "-"))
            {
                throw std::runtime_error(
                    "Flag '--out-dir' requires a directory path, but got flag '" + nextArg + "'.");
            }

            options.outputDir = StringUtils::ToWide(argv[++i]);
        }

        // ---- optimization level ----
        else if (StringUtils::StartsWith(arg, "-O"))
        {
            requireUnique("-O", arg);

            std::string levelStr = arg.substr(2);
            if (levelStr.empty())
            {
                throw std::runtime_error("Flag '-O' requires an optimization level (e.g. -O0, -O1).");
            }

            int level = 0;
            try
            {
                level = std::stoi(levelStr);
            }
            catch (...)
            {
                throw std::runtime_error("Invalid optimization level: '" + levelStr + "'.");
            }

            if (level < 0 || level > MAX_OPTIMIATION_LEVEL)
            {
                throw std::runtime_error(
                    "Optimization level must be between 0 and " +
                    std::to_string(MAX_OPTIMIATION_LEVEL) + ", got " +
                    std::to_string(level) + ".");
            }

            options.optimizationLevel = static_cast<short>(level);
        }

        // ---- unknown flag ----
        else if (StringUtils::StartsWith(arg, "-"))
        {
            throw std::runtime_error("Unknown flag: '" + arg + "'. Use --help for usage.");
        }

        // ---- positional argument (input file) ----
        else
        {
            options.inputFilesPath.push_back(StringUtils::ToWide(argv[i]));
        }
    }

    // ---- validation ----

    // --help and --version are standalone commands; reject if mixed with other flags/files
    if (options.displayHelp || options.displayVersion)
    {
        if (argc > 2)
        {
            throw std::runtime_error(
                "'--help' and '--version' are standalone commands. "
                "Do not combine with other flags.");
        }
        return options;
    }

    OptionsValidator::Validate(options);

    return options;
}

void CLIParser::PrintUsage()
{
    std::cout
        << "Usage: picoc [options] <file1.picoc> [file2.picoc ...]\n"
        << "\n"
        << "Options:\n"
        << "  --help          Show this help message and exit\n"
        << "  --version       Print compiler version and exit\n"
        << "\n"
        << "  -o <file>       Set output executable name (default: output)\n"
        << "  --out-dir <dir> Set output directory (default: current directory)\n"
        << "\n"
        << "  -O <level>       Set optimization level (0-" << MAX_OPTIMIATION_LEVEL << ", default: " << MAX_OPTIMIATION_LEVEL << ")\n"
        << "                    -O0  No optimizations\n"
        << "                    -O1  Basic optimizations\n"
        << "\n"
        << "  --warn-off      Disable all warnings\n"
        << "  --warn-all      Enable all warnings including pedantic\n"
        << "\n"
        << "  -v              Verbose output (print all compiler logs)\n"
        << "  -q              Quiet mode (suppress all logs)\n"
        << "  --save-logs     Save logs to a log file\n"
        << "\n"
        << "  --asm           Stop after assembly generation (.asm)\n"
        << "  --bin           Stop after assembler, produce object file (.obj)\n"
        << "  --save-temp     Keep all intermediate files (.asm, .obj, etc.)\n";
}

void CLIParser::PrintVersion()
{
    std::cout << "picoc compiler version 1.0.0\n";
}
