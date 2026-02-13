#include "includes/backend/codegen/assembler.h"

Assembler::Assembler(ErrorManager& errorManager) :
    m_errorManager(errorManager)
{ }

std::filesystem::path Assembler::Assemble(const std::string& asmContents, const std::string& programName)
{
    namespace fs = std::filesystem;

    fs::path outputDir = fs::current_path();
    fs::path asmFile = WriteAsmFile(asmContents, programName);
    if (asmFile.empty()) return fs::path{}; // error already stored

    // Determine NASM executable and format
    std::string objExt, nasmFormat;
    fs::path nasmPath;
    fs::path toolsDir; // temporary

    switch (TargetArchitecture::GetOS())
    {
        case OS::Windows:
            objExt = ".obj";
            nasmFormat = TargetArchitecture::Is64Bit() ? "win64" : "win32";

            toolsDir = SystemUtils::BuildRelativePath(L"tools/nasm");
            nasmPath = TargetArchitecture::Is64Bit()
                ? toolsDir / "nasm64.exe"
                : toolsDir / "nasm32.exe";

            if (!ValidateNasmPath(nasmPath)) return fs::path{}; // error already stored
            break;

        case OS::Linux:
            objExt = ".o";
            nasmFormat = TargetArchitecture::Is64Bit() ? "elf64" : "elf32";
            nasmPath = "nasm"; // assume installed
            break;

        default:
            this->m_errorManager.StoreError(CompileTimeError(
                ErrorTypes::INTERNAL_ERROR,
                "Unsupported OS for NASM backend",
                0, 0, "")
            );
            return fs::path{};
    }

    fs::path objFile = outputDir / (programName + objExt);

    // Build command and run
    std::string cmd = BuildNasmCommand(nasmPath, asmFile, objFile, nasmFormat);
    int result = std::system(cmd.c_str());
    if (result != 0)
    {
        this->m_errorManager.StoreError(CompileTimeError(
            ErrorTypes::INTERNAL_ERROR,
            "NASM failed with exit code: " + std::to_string(result),
            0, 0, "")
        );
        fs::remove(asmFile);
        return fs::path{};
    }

    return objFile;
}

std::filesystem::path Assembler::WriteAsmFile(const std::string& asmContents, const std::string& programName)
{
    namespace fs = std::filesystem;
    fs::path asmFile = fs::current_path() / (programName + ".asm");

    std::ofstream out(asmFile, std::ios::binary);
    if (!out)
    {
        this->m_errorManager.StoreError(CompileTimeError(
            ErrorTypes::INTERNAL_ERROR,
            "Failed to create ASM file",
            0, 0, "")
        );
        return fs::path{};
    }

    out << asmContents;
    out.close();
    return asmFile;
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
        // Wrap in cmd /C for Windows
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
        this->m_errorManager.StoreError(CompileTimeError(
            ErrorTypes::INTERNAL_ERROR,
            "NASM executable not found at: " + nasmPath.string(),
            0, 0, "")
        );
        return false;
    }
    return true;
}
