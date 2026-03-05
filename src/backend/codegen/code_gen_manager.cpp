#include "includes/backend/codegen/code_gen_manager.h"
#include "includes/shared/utils/string_utils.hpp"

CodeGenManager::CodeGenManager(const DecoratedAST& decoratedAST, const CompilerOptions& options) :
    m_decoratedAST(decoratedAST),
    m_options(options),
    m_errorManager(ErrorManager::GetInstance())
{}

std::string CodeGenManager::MainFlow()
{
    IRGenerator irGenerator(this->m_decoratedAST);
    IRModule& irProgram = irGenerator.MainFlow();

    if (this->m_options.optimizationLevel > 0)
    {
        OptimizationManager optimizationManager;
        optimizationManager.MainFlow(irProgram);
    }

    std::string outputName = StringUtils::ToNarrow(this->m_options.outputFileName);
    std::string outputDir  = StringUtils::ToNarrow(this->m_options.outputDir);

    ASMGenerator asmGenerator(irProgram, this->m_decoratedAST.funcTable, outputName, outputDir);
    std::string asmFilePath = asmGenerator.MainFlow();

    if (this->m_options.stopAtAsm)
    {
        return asmFilePath;
    }

    Assembler assembler;
    std::string objectFilePath = assembler.Mainflow(asmFilePath, outputName, outputDir);

    if (!this->m_options.saveTemps)
    {
        std::filesystem::remove(asmFilePath);
    }

    return objectFilePath;
}
