#include "includes/compiler_orchestrator.h"
#include "includes/shared/logger/console.h"
#include "includes/shared/utils/string_utils.hpp"

CompilerOrchestrator::CompilerOrchestrator(CompilerOptions options) :
    m_options(std::move(options))
{}

void CompilerOrchestrator::MainFlow()
{
    // ---- Resolve output directory once, so every downstream component uses it ----
    if (m_options.outputDir.empty())
    {
        m_options.outputDir = std::filesystem::current_path().wstring();
    }
    else
    {
        // make sure the user-supplied dir is absolute
        std::filesystem::path dir(m_options.outputDir);
        if (dir.is_relative())
        {
            dir = std::filesystem::current_path() / dir;
        }
        // create the directory if it doesn't exist
        std::filesystem::create_directories(dir);
        m_options.outputDir = dir.wstring();
    }

    Console& console = Console::GetInstance();
    // Always set explicitly — Console is a singleton that persists across
    // multiple compilations in the GUI, so we must reset every time.
    console.SetVerbosity(m_options.isVerbose ? Verbosity::VERBOSE : Verbosity::QUIET);
    console.SetWarningLevel(m_options.warningLevel);
    if (m_options.saveLogs)
    {
        std::filesystem::path logPath = std::filesystem::path(m_options.outputDir)
            / (StringUtils::ToNarrow(m_options.outputFileName) + ".log");
        console.EnableFileLogging(logPath.string());
    }

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
