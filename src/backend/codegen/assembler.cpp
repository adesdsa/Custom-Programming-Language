#include "includes/backend/codegen/assembler.h"

Assembler::Assembler(ErrorManager& errorManager) :
    m_errorManager(errorManager)
{ }

std::filesystem::path Assembler::Assemble(const std::string& asmContents)
{
    namespace fs = std::filesystem;
    std::string uniquePath =
        std::to_string(
            std::chrono::high_resolution_clock::now()
            .time_since_epoch()
            .count());

    // ---------- Generate unique temp filenames ----------
    fs::path tempDir = fs::temp_directory_path();
    fs::path asmFile = tempDir / (uniquePath + ".asm");

    std::string objExt;
    std::string nasmFormat;

    // ---------- Determine NASM format ----------
    if (TargetArchitecture::GetOS() == OS::Windows)
    {
        objExt = ".obj";
        nasmFormat = TargetArchitecture::Is64Bit() ? "win64" : "win32";
    }
    else if (TargetArchitecture::GetOS() == OS::Linux)
    {
        objExt = ".o";
        nasmFormat = TargetArchitecture::Is64Bit() ? "elf64" : "elf32";
    }
    else
    {
        this->m_errorManager.StoreError(CompileTimeError(ErrorTypes::INTERNAL_ERROR,
            "Unsupported OS for NASM backend",
            0,
            0,
            "")
        );
        return fs::path{};
    }

    fs::path outputDir = fs::current_path();
    fs::path objFile = outputDir / (uniquePath + objExt);

    // ---------- Write ASM file ----------
    std::ofstream out(asmFile, std::ios::binary);
    if (!out)
    {
        this->m_errorManager.StoreError(CompileTimeError(ErrorTypes::INTERNAL_ERROR,
            "Failed to create temporary ASM file",
            0,
            0,
            "")
        );
        fs::remove(asmFile);
        return fs::path{};
    }

    out << asmContents;
    out.close();

    // ---------- Build NASM command ----------
    std::ostringstream cmd;
    cmd << "nasm -f " << nasmFormat
        << " \"" << asmFile.string() << "\""
        << " -o \"" << objFile.string() << "\"";

    int result = std::system(cmd.str().c_str());

    if (result != 0)
    {
        this->m_errorManager.StoreError(CompileTimeError(ErrorTypes::INTERNAL_ERROR,
            "NASM failed with exit code: " + std::to_string(result),
            0,
            0,
            "")
        );
        fs::remove(asmFile);
        return fs::path{};
    }

    fs::remove(asmFile);
    return objFile;
}