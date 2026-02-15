#include "includes/backend/linker/linker.h"

Linker::Linker() :
    m_logger("LINKER")
{
    this->m_logger.Info("Linker component initialized successfully");
}

std::string Linker::MainFlow(const std::string& objFilePath)
{
    // verify the object file exists
    if (!std::filesystem::exists(objFilePath))
    {
        this->m_logger.Error("Object file not found: " + objFilePath);
        throw std::runtime_error("Linker failed: object file not found");
    }

    std::string exePath = this->DeriveOutputPath(objFilePath);
    exePath = std::filesystem::absolute(exePath).string();
    std::string command = this->BuildLinkerCommand(objFilePath, exePath);

    this->m_logger.Info("Linking: " + command);

    std::string output;
    int exitCode = SystemUtils::RunProcess(command, output);

    if (exitCode != OK_EXIT_CODE)
    {
        this->m_logger.Error("Linker failed (exit code " + std::to_string(exitCode) + ")");

        if (!output.empty())
        {
            this->m_logger.Error("Linker output:\n" + output);
        }

        throw std::runtime_error("Linking failed with exit code " + std::to_string(exitCode));
    }

    if (!output.empty())
    {
        this->m_logger.Info("Linker output: " + output);
    }

    this->m_logger.Info("Linked successfully: " + exePath);
    return exePath;
}

std::string Linker::BuildLinkerCommand(
    const std::string& objPath,
    const std::string& exePath) const
{
    if constexpr (TargetArchitecture::GetOS() == OS::Windows)
    {
        std::string golinkPath = std::filesystem::absolute(GOLINK_TOOL_PATH).string();
        std::string entryPoint;
    
        if constexpr (TargetArchitecture::Is64Bit())
        {
            entryPoint = "mainCRTStartup";
        }
        else
        {
            entryPoint = "_mainCRTStartup";
        }

        /**
            * /console  -> produce a console application
            * /entry    -> set the entry point symbol
            * kernel32.dll -> resolve ExitProcess and other WinAPIs
            *
            * The inner command is wrapped with cmd /C "..." so that
            * _popen (which itself invokes cmd /C) does not strip the
            * leading/trailing quotes from the executable path — this
            * is required when the path contains spaces.
        */
        std::string inner =
            "\"" + golinkPath + "\""
            + " /console"
            + " /entry " + entryPoint
            + " /fo \"" + exePath + "\""
            + " \"" + objPath + "\""
            + " kernel32.dll"
            + " msvcrt.dll";

        return "cmd /C \"" + inner + "\"";
    }
    else if constexpr (TargetArchitecture::GetOS() == OS::Linux)
    {
        if constexpr (TargetArchitecture::Is64Bit())
        {
            return "ld -o \"" + exePath + "\" \"" + objPath + "\"";
        }
        else
        {
            return "ld -m elf_i386 -o \"" + exePath + "\" \"" + objPath + "\"";
        }
    }
    else
    {
        throw std::runtime_error("Unsupported OS for linking");
    }
}

std::string Linker::DeriveOutputPath(const std::string& objPath) const
{
    std::filesystem::path path(objPath);

    if constexpr (TargetArchitecture::GetOS() == OS::Windows)
    {
        return (path.parent_path() / path.stem()).string() + ".exe";
    }
    else
    {
        // linux has no extension for executables
        return (path.parent_path() / path.stem()).string();
    }
}
