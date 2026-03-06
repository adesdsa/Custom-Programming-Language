#include "includes/backend/linker/linker.h"

Linker::Linker() :
    m_logger("LINKER")
{
    this->m_logger.Info("Linker component initialized successfully");
}

std::string Linker::MainFlow(
    const std::vector<std::string>& objFilePaths,
    const std::string& outputName,
    const std::string& outputDir)
{
    if (objFilePaths.empty())
    {
        this->m_logger.Error("No object files provided");
        throw std::runtime_error("Linker failed: no object files provided");
    }

    for (const auto& objPath : objFilePaths)
    {
        if (!std::filesystem::exists(objPath))
        {
            this->m_logger.Error("Object file not found: " + objPath);
            throw std::runtime_error("Linker failed: object file not found: " + objPath);
        }
    }

    std::string exePath = this->DeriveOutputPath(outputName, outputDir);
    std::string command = this->BuildLinkerCommand(objFilePaths, exePath);

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
    const std::vector<std::string>& objPaths,
    const std::string& exePath) const
{
    #if defined(_WIN32) || defined(_WIN64)

        std::string golinkPath = SystemUtils::BuildRelativePath(L"tools/golink/GoLink.exe").string();
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
            + " /fo \"" + exePath + "\"";

        for (const auto& objPath : objPaths)
        {
            inner += " \"" + objPath + "\"";
        }

        /* /mix: allow mixing object formats if needed */
        inner += " /mix";

        inner += " kernel32.dll"
              +  std::string(" msvcrt.dll");

        return "cmd /C \"" + inner + "\"";

    #elif defined(__linux__)

        std::string cmd;

        if constexpr (TargetArchitecture::Is64Bit())
        {
            cmd = "ld -o \"" + exePath + "\"";
        }
        else
        {
            cmd = "ld -m elf_i386 -o \"" + exePath + "\"";
        }

        for (const auto& objPath : objPaths)
        {
            cmd += " \"" + objPath + "\"";
        }

        cmd += " -lc";
        return cmd;

    #else
        throw std::runtime_error("Unsupported OS for linking");
    #endif
}

std::string Linker::DeriveOutputPath(
    const std::string& outputName,
    const std::string& outputDir) const
{
    std::filesystem::path dir(outputDir);
    std::filesystem::path exePath;

    if constexpr (TargetArchitecture::GetOS() == OS::Windows)
    {
        exePath = dir / (outputName + ".exe");
    }
    else
    {
        exePath = dir / outputName;
    }

    return std::filesystem::absolute(exePath).string();
}
