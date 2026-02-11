#include "includes/backend/codegen/linker.h"

Linker::Linker(ErrorManager& errorManager)
    : m_errorManager(errorManager)
{ }

std::filesystem::path Linker::Link(const std::filesystem::path& objFilePath)
{
    namespace fs = std::filesystem;

    if (!fs::exists(objFilePath))
    {
        m_errorManager.StoreError(CompileTimeError(
            ErrorTypes::INTERNAL_ERROR,
            "Object file does not exist: " + objFilePath.string(),
            0,
            0,
            "")
        );
        return {};
    }

    std::string exeExt;
    std::ostringstream cmd;
    fs::path exeFile;

    if (TargetArchitecture::GetOS() == OS::Windows)
    {
        exeExt = ".exe";
        exeFile = fs::current_path() / (objFilePath.stem().string() + exeExt);

        cmd << "link /NOLOGO /OUT:\"" << exeFile.string()
            << "\" \"" << objFilePath.string() << "\"";
    }
    else if (TargetArchitecture::GetOS() == OS::Linux)
    {
        exeExt = ".elf";
        exeFile = fs::current_path() / (objFilePath.stem().string() + exeExt);

        // Basic Linux linker
        cmd << "ld -o \"" << exeFile.string()
            << "\" \"" << objFilePath.string() << "\"";
    }
    else
    {
        m_errorManager.StoreError(CompileTimeError(
            ErrorTypes::INTERNAL_ERROR,
            "Unsupported OS for linking",
            0,
            0,
            "")
        );
        return {};
    }

    int result = std::system(cmd.str().c_str());

    if (result != 0)
    {
        m_errorManager.StoreError(CompileTimeError(
            ErrorTypes::INTERNAL_ERROR,
            "Linker failed with exit code: " + std::to_string(result),
            0,
            0,
            "")
        );
        return {};
    }

    return exeFile;
}
