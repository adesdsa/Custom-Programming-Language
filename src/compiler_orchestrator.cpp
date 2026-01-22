#include "includes/compiler_orchestrator.h"
#include "includes/backend/codegen/ir/ir_generator.h"

CompilerOrchestrator::CompilerOrchestrator(int argc, const char* argv[]) :
    m_filePath(DesriralizeCLI(argc, argv))
{}

void CompilerOrchestrator::MainFlow()
{
    ParserLogic parserComponent(
        SystemUtils::BuildRelativePath(this->m_filePath)
    );

    const AST& ast = parserComponent.MainFlow();
    ast.PrintAST();

    SemanticAnalysisLogicComponent semanticAnalysisComponent(ast);
    const DecoratedAST& decoratedAST = semanticAnalysisComponent.MainFlow();

    CodeGenManager codeGenManager(decoratedAST);
    (void)codeGenManager.MainFlow();
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