#include "includes/compiler_orchestrator.h"
#include "includes/shared/logger/console.h"
#include "includes/shared/logger/entity_logger.h"
#include "includes/shared/errors/error_manager.h"
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

    // Step 1: Parse all source files (sequential — lexer/parser is fast)
    std::vector<std::unique_ptr<ParserLogic>> parsers;
    std::vector<const AST*> asts;

    for (const auto& filePath : this->m_options.inputFilesPath)
    {
        auto parser = std::make_unique<ParserLogic>(filePath);
        const AST& ast = parser->MainFlow();
        asts.push_back(&ast);
        parsers.push_back(std::move(parser));
    }

    // Step 2: Collect exports from each AST into the project-wide symbol table (sequential)
    ProjectSymbolTable projectSymbols;
    for (size_t i = 0; i < parsers.size(); i++)
    {
        std::filesystem::path inputPath(this->m_options.inputFilesPath[i]);
        std::string fileName = inputPath.stem().string();
        std::string filePathNarrow = inputPath.string();

        FileExportTable exports = CollectExports(*asts[i], filePathNarrow);
        projectSymbols.Register(fileName, std::move(exports));
    }

    // Step 3: Parallel semantic analysis + code generation
    size_t hwThreads = std::thread::hardware_concurrency();
    size_t maxThreads = std::min(parsers.size(), hwThreads);
    ThreadPool pool(maxThreads);

    std::vector<std::future<std::string>> futures;
    for (size_t i = 0; i < parsers.size(); i++)
    {
        futures.push_back(pool.Enqueue([this, i, &asts, &projectSymbols]() -> std::string
        {
            std::filesystem::path inputPath(this->m_options.inputFilesPath[i]);
            std::string perFileName = inputPath.stem().string();

            SemanticAnalysisLogicComponent semantic(*asts[i], projectSymbols);
            const DecoratedAST& decorated = semantic.MainFlow();

            CompilerOptions perFileOptions = this->m_options;
            perFileOptions.outputFileName = std::wstring(perFileName.begin(), perFileName.end());

            CodeGenManager codegen(decorated, perFileOptions);
            return codegen.MainFlow();
        }));
    }

    // Step 4: Collect all artifact paths (blocks until each job completes)
    std::vector<std::string> artifactPaths;
    for (auto& future : futures)
    {
        artifactPaths.push_back(future.get());
    }

    // Step 5: Check for errors accumulated across all threads
    if (ErrorManager::GetInstance().HasErrorsOccured())
    {
        ErrorManager::GetInstance().HandleErrors();
    }

    // Step 6: Link all object files into the final executable
    if (!this->m_options.stopAtBin && !this->m_options.stopAtAsm)
    {
        std::string outputName = StringUtils::ToNarrow(this->m_options.outputFileName);
        std::string outputDir  = StringUtils::ToNarrow(this->m_options.outputDir);

        Linker linker;
        linker.MainFlow(artifactPaths, outputName, outputDir);

        if (!this->m_options.saveTemps)
        {
            for (const auto& objPath : artifactPaths)
            {
                std::filesystem::remove(objPath);
            }
        }
    }
}

