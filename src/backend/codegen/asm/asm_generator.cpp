#include "includes/backend/codegen/asm/asm_generator.h"
#include "includes/backend/codegen/asm/targets/x86_emitter.h"
#include "includes/backend/codegen/asm/targets/x86_64emitter.h"

#include <fstream>
#include <stdexcept>

ASMGenerator::ASMGenerator(const IRModule& program, const FuncTable& funcTable) :
    m_irMoudle(program),
    m_funcTable(funcTable),
    m_ASMLogger("ASM"),
    m_errorManager(ErrorManager::GetInstance())
{
    this->m_ASMLogger.Info("ASMGenerator component initialized successfully");
}

std::string ASMGenerator::MainFlow()
{
    this->m_ASMLogger.Info("Selecting target emitter for host architecture");
    std::unique_ptr<ITargetEmitter> emitter = this->CreateEmitter();

    this->m_ASMLogger.Info("Start generating ASM");
    std::string asmContent = emitter->Emit(this->m_irMoudle);

    std::string fileExtension = emitter->GetFileExtension();
    std::string outputPath = this->WriteAsmToFile(asmContent, fileExtension);

    this->m_ASMLogger.Info("Assembly written to: " + outputPath);
    return outputPath;
}

std::unique_ptr<ITargetEmitter> ASMGenerator::CreateEmitter() const
{
    switch (TargetArchitecture::GetArch())
    {
        case BitArchitecture::X86:
            this->m_ASMLogger.Info("Detected Target: x86 (32-bit)");
            return std::make_unique<X86_Emitter>(this->m_funcTable);

        case BitArchitecture::X86_64:
            this->m_ASMLogger.Info("Detected Target: x86_64 (64-bit)");
            return std::make_unique<X86_64Emitter>(this->m_funcTable);

        case BitArchitecture::ARM32:
        case BitArchitecture::ARM64:
            throw std::runtime_error("ARM architecture is not yet supported");

        default:
            throw std::runtime_error("Unknown target architecture");
    }
}

std::string ASMGenerator::WriteAsmToFile(
    const std::string& asmContent, 
    const std::string& fileExtension
) const
{
    // build output path: /build/out.asm
    std::filesystem::path outputDir = std::filesystem::path(PROJECT_ROOT) / "build";
    std::filesystem::create_directories(outputDir);

    std::string outputFileName = "out" + fileExtension;
    std::filesystem::path outputPath = outputDir / outputFileName;

    std::ofstream outFile(outputPath);
    if (!outFile.is_open())
    {
        throw std::runtime_error("Failed to open output file: " + outputPath.string());
    }

    outFile << asmContent;
    outFile.close();

    return outputPath.string();
}
