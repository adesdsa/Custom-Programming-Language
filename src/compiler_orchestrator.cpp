#include "includes/compiler_orchestrator.h"
#include "includes/shared/logger/console.h"
#include "includes/shared/logger/entity_logger.h"
#include "includes/shared/utils/string_utils.hpp"

CompilerOrchestrator::CompilerOrchestrator(CompilerOptions options) :
    m_options(std::move(options))
{}

void CompilerOrchestrator::Init()
{
    // ---- if -o contains a path (e.g. "dir1/dir2"), split into dir + name ----
    {
        std::filesystem::path outputAsPath(m_options.outputFileName);
        if (outputAsPath.has_parent_path())
        {
            // merge the directory part into outputDir
            std::filesystem::path extraDir = outputAsPath.parent_path();
            if (this->m_options.outputDir.empty())
            {
                this->m_options.outputDir = extraDir.wstring();
            }
            else
            {
                this->m_options.outputDir = (
                    std::filesystem::path(m_options.outputDir) / extraDir
                ).wstring();
            }
            // keep only the bare file name
            m_options.outputFileName = outputAsPath.filename().wstring();
        }
    }

    // ---- resolve output directory once, so every downstream component uses it ----
    if (this->m_options.outputDir.empty())
    {
        std::filesystem::path defaultDir = std::filesystem::current_path();

        if (SystemUtils::IsDirectoryWritable(defaultDir))
        {
            this->m_options.outputDir = defaultDir.wstring();
        }
        else if (!this->m_options.inputFilesPath.empty())
        {
            // CWD is not writable – try the input file's directory instead
            std::filesystem::path inputDir =
                std::filesystem::absolute(
                    std::filesystem::path(this->m_options.inputFilesPath[0]).parent_path());

            if (SystemUtils::IsDirectoryWritable(inputDir))
            {
                EntityLogger logger("GENERAL");
                logger.Warn(
                    "No write permission to default output directory \""
                    + defaultDir.string()
                    + "\". Falling back to input file directory \""
                    + inputDir.string() + "\".");
                this->m_options.outputDir = inputDir.wstring();
            }
            else
            {
                throw std::runtime_error(
                    "Permission denied: cannot write to output directory \""
                    + defaultDir.string()
                    + "\" or input file directory \""
                    + inputDir.string()
                    + "\". Please run as administrator or choose a different output directory.");
            }
        }
        else
        {
            throw std::runtime_error(
                "Permission denied: cannot write to default output directory \""
                + defaultDir.string()
                + "\". Please run as administrator or choose a different output directory.");
        }
    }
    else
    {
        // make sure the user supplied dir is absolute
        std::filesystem::path dir(m_options.outputDir);
        if (dir.is_relative())
        {
            dir = std::filesystem::current_path() / dir;
        }
        // create the directory if it doesn't exist
        std::filesystem::create_directories(dir);
        m_options.outputDir = dir.wstring();
    }

    // ---- configure the logger singleton ----
    Console& console = Console::GetInstance();
    console.SetVerbosity(m_options.isVerbose ? Verbosity::VERBOSE : Verbosity::QUIET);
    console.SetWarningLevel(m_options.warningLevel);

    if (m_options.saveLogs)
    {
        std::filesystem::path logPath = std::filesystem::path(m_options.outputDir)
            / (StringUtils::ToNarrow(m_options.outputFileName) + ".log");
        console.EnableFileLogging(logPath.string());
    }

    // ---- resolve input file paths ----
    for (auto& inputPath : this->m_options.inputFilesPath)
    {
        std::filesystem::path filePath(inputPath);
        if (filePath.is_relative())
        {
            filePath = std::filesystem::current_path() / filePath;
        }
        inputPath = filePath.wstring();
    }
}

void CompilerOrchestrator::MainFlow()
{
    this->Init();

    // TODO: Support multi-file compilation for now compile the first file
    std::filesystem::path filePath(this->m_options.inputFilesPath[0]);

    ParserLogic parserComponent(filePath);

    const AST& ast = parserComponent.MainFlow();

    SemanticAnalysisLogicComponent semanticAnalysisComponent(ast);
    const DecoratedAST& decoratedAST = semanticAnalysisComponent.MainFlow();

    CodeGenManager codeGenManager(decoratedAST, this->m_options);
    codeGenManager.MainFlow();
}
