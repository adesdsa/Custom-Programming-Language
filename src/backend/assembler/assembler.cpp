#include "includes/backend/assembler/assembler.h"
#include <stdexcept>

Assembler::Assembler(ErrorManager& errorManager) :
    m_assemblerLogger("Assembler")
{ }

std::string Assembler::Mainflow(const std::string& asmPath, const std::string& programName)
{
    namespace fs = std::filesystem;

    fs::path outputDir = fs::current_path();
    fs::path asmFile = asmPath;

    if (!fs::exists(asmFile))
    {
        this->m_assemblerLogger.Error("ASM file does not exist: " + asmFile.string());
        throw std::runtime_error("");
    }

    std::string objExt, nasmFormat;
    fs::path nasmPath;
    fs::path toolsDir;

    switch (TargetArchitecture::GetOS())
    {
        case OS::Windows:
            objExt = ".obj";
            nasmFormat = TargetArchitecture::Is64Bit() ? "win64" : "win32";

            toolsDir = SystemUtils::BuildRelativePath(L"tools/nasm");
            nasmPath = TargetArchitecture::Is64Bit()
                ? toolsDir / "nasm64.exe"
                : toolsDir / "nasm32.exe";

            if (!ValidateNasmPath(nasmPath))
                throw std::runtime_error("");
            break;

        case OS::Linux:
            objExt = ".o";
            nasmFormat = TargetArchitecture::Is64Bit() ? "elf64" : "elf32";
            nasmPath = "nasm";
            break;

        default:
        {
            this->m_assemblerLogger.Error("Unsupported OS for NASM backend");
            throw std::runtime_error("");
        }
    }

    fs::path objFile = outputDir / (programName + objExt);

    std::string cmd = BuildNasmCommand(nasmPath, asmFile, objFile, nasmFormat);

    this->m_logger.Info("Assembling: " + cmd);

    int result = std::system(cmd.c_str());

    if (result != 0)
    {
        this->m_assemblerLogger.Error("NASM failed with exit code: " + std::to_string(result));
        throw std::runtime_error("");
    }

    this->m_logger.Info("Assembled successfully: " + objFile.string());

    return objFile.string();
}

std::string Assembler::BuildNasmCommand(
    const std::filesystem::path& nasmPath,
    const std::filesystem::path& asmFile,
    const std::filesystem::path& objFile,
    const std::string& nasmFormat) const
{
    std::ostringstream cmd;

    if (TargetArchitecture::GetOS() == OS::Windows)
    {
        std::ostringstream inner;
        inner << "\"" << nasmPath.string() << "\""
              << " -f " << nasmFormat
              << " \"" << asmFile.string() << "\""
              << " -o \"" << objFile.string() << "\"";

        cmd << "cmd /C \"" << inner.str() << "\"";
    }
    else
    {
        cmd << "\"" << nasmPath.string() << "\""
            << " -f " << nasmFormat
            << " \"" << asmFile.string() << "\""
            << " -o \"" << objFile.string() << "\"";
    }

    return cmd.str();
}

bool Assembler::ValidateNasmPath(const std::filesystem::path& nasmPath)
{
    namespace fs = std::filesystem;

    if (!fs::exists(nasmPath))
    {
        this->m_assemblerLogger.Error("NASM executable not found at: " + nasmPath.string();
        throw std::runtime_error("");
    }

    return true;
}
