#include "includes/compiler_orchestrator.h"
#include "includes/shared/logger/console.h"
#include "includes/shared/logger/entity_logger.h"
#include "includes/shared/errors/error_manager.h"
#include "includes/shared/errors/error_models.h"
#include "includes/shared/utils/string_utils.hpp"
#include "includes/shared/utils/system_utils.h"
#include "includes/shared/common/target_architecture.h"
#include "includes/shared/common/builtin_library.h"

#include <algorithm>
#include <future>
#include <set>
#include <thread>
#include <unordered_map>
#include <unordered_set>

namespace
{
    struct FileArtifactResult
    {
        std::string objectPath;
        std::set<std::string> requiredLibraries;
    };
}

CompilerOrchestrator::CompilerOrchestrator(CompilerOptions options) :
    m_options(std::move(options))
{}

void CompilerOrchestrator::MainFlow()
{
    this->Init();

    ErrorManager& errorManager = ErrorManager::GetInstance();
    errorManager.Clear();
    errorManager.SetDeferredMode(true);

    // Phase 1: Parse all source files
    std::vector<std::unique_ptr<ParserLogic>> parsers;
    std::vector<const AST*> asts;
    this->ParseSourceFiles(parsers, asts);

    // Abort early if any syntax errors occurred during parsing
    if (errorManager.HasErrorsOccured())
    {
        errorManager.SetDeferredMode(false);
        errorManager.HandleErrors();
        return;
    }

    // Phase 2: Detect circular imports (abort early if found)
    if (this->DetectCircularImports(asts))
    {
        errorManager.SetDeferredMode(false);
        if (errorManager.HasErrorsOccured())
        {
            errorManager.HandleErrors();
        }
        return;
    }

    ProjectSymbolTable projectSymbols = this->CollectProjectExports(asts);

    std::vector<std::string> artifactPaths;
    std::set<std::string> requiredLibraries;
    this->AnalyzeAndGenerate(asts, projectSymbols, artifactPaths, requiredLibraries);

    errorManager.SetDeferredMode(false);
    if (errorManager.HasErrorsOccured())
    {
        errorManager.HandleErrors();
    }

    if (!this->m_options.stopAtBin && !this->m_options.stopAtAsm)
    {
        this->LinkExecutable(artifactPaths, requiredLibraries);
    }
}

void CompilerOrchestrator::ParseSourceFiles(
    std::vector<std::unique_ptr<ParserLogic>>& parsers,
    std::vector<const AST*>& asts)
{
    for (const auto& filePath : this->m_options.inputFilesPath)
    {
        std::filesystem::path fsPath(filePath);
        ErrorManager::SetCurrentFilePath(fsPath.string());

        auto parser = std::make_unique<ParserLogic>(filePath);
        const AST& ast = parser->MainFlow();
        asts.push_back(&ast);
        parsers.push_back(std::move(parser));
    }
}

bool CompilerOrchestrator::DetectCircularImports(
    const std::vector<const AST*>& asts) const
{
    std::unordered_map<std::string, std::vector<std::string>> importGraph;
    std::unordered_set<std::string> fileStems;

    for (size_t i = 0; i < asts.size(); i++)
    {
        std::filesystem::path inputPath(this->m_options.inputFilesPath[i]);
        std::string stem = inputPath.stem().string();
        fileStems.insert(stem);
        importGraph[stem] = CollectImports(*asts[i]);
    }

    // DFS cycle detection (white=0, gray=1, black=2)
    std::unordered_map<std::string, int> color;
    std::vector<std::string> stack;
    std::string cycleStart;

    auto dfs = [&](const std::string& node, auto& self) -> bool 
    {
        if (color[node] == 1) 
        { 
            cycleStart = node; 
            return true; 
        }

        else if (color[node] == 2) 
        {
            return false;
        }

        color[node] = 1;
        stack.push_back(node);
        for (const auto& imp : importGraph[node])
        {
            if (fileStems.count(imp) == 0) 
            {
                continue;
            }
            if (self(imp, self)) 
            {
                return true;
            }
        }
        stack.pop_back();
        color[node] = 2;
        return false;
    };

    for (const auto& [stem, _] : importGraph)
    {
        if (color[stem] != 0) 
        {
            continue;
        }
        stack.clear();

        if (dfs(stem, dfs))
        {
            auto it = std::find(stack.begin(), stack.end(), cycleStart);
            std::string cyclePath;
            for (; it != stack.end(); it++)
            {
                if (!cyclePath.empty()) 
                {
                    cyclePath += " -> ";
                }
                cyclePath += *it;
            }
            cyclePath += " -> " + cycleStart;

            ErrorManager::SetCurrentFilePath(
                StringUtils::ToNarrow(this->m_options.inputFilesPath[0]));
            ErrorManager::GetInstance().StoreError(CompileTimeError(
                ErrorTypes::CIRCULAR_IMPORT,
                "Circular import detected: " + cyclePath,
                0, 0,
                "Remove the circular dependency between these files"
            ));
            return true;
        }
    }
    return false;
}

ProjectSymbolTable CompilerOrchestrator::CollectProjectExports(
    const std::vector<const AST*>& asts) const
{
    ProjectSymbolTable projectSymbols;
    for (size_t i = 0; i < asts.size(); i++)
    {
        std::filesystem::path inputPath(this->m_options.inputFilesPath[i]);
        std::string fileName  = inputPath.stem().string();
        std::string filePath  = inputPath.string();

        FileExportTable exports = CollectExports(*asts[i], filePath);
        projectSymbols.Register(fileName, std::move(exports));
    }
    return projectSymbols;
}

void CompilerOrchestrator::AnalyzeAndGenerate(
    const std::vector<const AST*>& asts,
    const ProjectSymbolTable& projectSymbols,
    std::vector<std::string>& artifactPaths,
    std::set<std::string>& requiredLibraries)
{
    if (asts.empty())
        return;

    size_t maxThreads = std::min(
        asts.size(),
        static_cast<size_t>(std::thread::hardware_concurrency()));
    if (maxThreads == 0)
        maxThreads = 1;

    ThreadPool pool(maxThreads);

    std::vector<std::future<FileArtifactResult>> futures;
    futures.reserve(asts.size());

    for (size_t i = 0; i < asts.size(); i++)
    {
        futures.push_back(pool.Enqueue([this, &asts, &projectSymbols, i]() -> FileArtifactResult
        {
            std::filesystem::path inputPath(this->m_options.inputFilesPath[i]);
            std::string perFileName = inputPath.stem().string();

            ErrorManager::SetCurrentFilePath(inputPath.string());

            SemanticAnalysisLogicComponent semantic(*asts[i], projectSymbols);
            semantic.MainFlow();

            // If semantic analysis reported errors for this file only, skip IR/codegen for this file.
            // Other files continue (per-file check, not global).
            std::string thisFilePath = inputPath.string();
            if (ErrorManager::GetInstance().HasErrorsForFile(thisFilePath))
            {
                return FileArtifactResult{};
            }

            const DecoratedAST& decorated = semantic.GetDecoratedAST();

            CompilerOptions perFileOptions = this->m_options;
            perFileOptions.outputFileName = std::wstring(perFileName.begin(), perFileName.end());

            CodeGenManager codegen(decorated, perFileOptions);
            std::string objectPath = codegen.MainFlow();

            FileArtifactResult result;
            result.objectPath = std::move(objectPath);
            result.requiredLibraries = codegen.GetRequiredLibraries();
            return result;
        }));
    }

    artifactPaths.clear();
    artifactPaths.reserve(asts.size());
    requiredLibraries.clear();

    pool.WaitAll();

    try
    {
        for (size_t i = 0; i < futures.size(); i++)
        {
            FileArtifactResult result = futures[i].get();
            if (!result.objectPath.empty())
            {
                artifactPaths.push_back(std::move(result.objectPath));
            }
            for (const auto& lib : result.requiredLibraries)
                requiredLibraries.insert(lib);
        }
    }
    catch (...)
    {
        ErrorManager::GetInstance().SetDeferredMode(false);
        if (ErrorManager::GetInstance().HasErrorsOccured())
        {
            ErrorManager::GetInstance().HandleErrors();
        }
        throw;
    }
}

void CompilerOrchestrator::LinkExecutable(
    std::vector<std::string>& artifactPaths,
    const std::set<std::string>& requiredLibraries)
{
    std::string outputName = StringUtils::ToNarrow(this->m_options.outputFileName);
    std::string outputDir  = StringUtils::ToNarrow(this->m_options.outputDir);

    size_t userArtifactCount = artifactPaths.size();
    for (const auto& libName : requiredLibraries)
    {
        std::string libObjPath = BuiltinLibraryRegistry::GetObjPath(libName);
        if (std::filesystem::exists(libObjPath))
            artifactPaths.push_back(libObjPath);
    }

    Linker linker;
    linker.MainFlow(artifactPaths, outputName, outputDir);

    if (!this->m_options.saveTemps)
    {
        for (size_t i = 0; i < userArtifactCount; i++)
            std::filesystem::remove(artifactPaths[i]);
    }
}

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
