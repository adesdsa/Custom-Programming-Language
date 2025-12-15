#include "includes/compiler_orchestrator.h"

CompilerOrchestrator::CompilerOrchestrator(int argc, const char* argv[]) :
    m_filePath(DesriralizeCLI(argc, argv)),
    m_parserLogic(SystemUtils::BuildRelativePath(this->m_filePath)) {}

void CompilerOrchestrator::MainFlow()
{
    // TODO: Add semantic analysis logic call here
    const AST& ast = this->m_parserLogic.MainFlow();
    ast.PrintAST();
}

std::wstring CompilerOrchestrator::DesriralizeCLI(int argc, const char *argv[])
{
    // @note for now only compilation of single file supported, no flags too
    if (argc != 2)
    {
        throw std::runtime_error("ERROR: Supports only single file compilation for now...");
    }
    const char* filePath = argv[1];  // fow now assumes argv[1] is a valid file...
    return std::wstring(filePath, filePath + strlen(filePath));
}