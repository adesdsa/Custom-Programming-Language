#include "includes/compiler_orchestrator.h"
#include "includes/shared/logger/console.h"

CompilerOrchestrator::CompilerOrchestrator(int argc, const char* argv[]) :
    m_options(CLIParser::Parse(argc, argv))
{}

void CompilerOrchestrator::MainFlow()
{
    // Handle standalone commands before compilation
    if (m_options.displayHelp)
    {
        CLIParser::PrintUsage();
        return;
    }

    if (m_options.displayVersion)
    {
        CLIParser::PrintVersion();
        return;
    }

    Console& console = Console::GetInstance();
    if (m_options.isVerbose)
    {
        console.SetVerbosity(Verbosity::VERBOSE);
    }
    console.SetWarningLevel(m_options.warningLevel);

    // TODO: Support multi-file compilation; for now compile the first file
    std::filesystem::path filePath(m_options.inputFilesPath[0]);

    // resolve relative paths against the caller's working directory;
    // absolute paths (e.g. C:\...) are used as-is.
    if (filePath.is_relative())
    {
        filePath = std::filesystem::current_path() / filePath;
    }

    ParserLogic parserComponent(filePath);

    const AST& ast = parserComponent.MainFlow();

    SemanticAnalysisLogicComponent semanticAnalysisComponent(ast);
    const DecoratedAST& decoratedAST = semanticAnalysisComponent.MainFlow();

    CodeGenManager codeGenManager(decoratedAST, m_options);
    codeGenManager.MainFlow();
}
