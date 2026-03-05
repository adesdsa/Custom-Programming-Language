#include "includes/compiler_orchestrator.h"
#include "includes/shared/logger/console.h"
#include "includes/shared/logger/entity_logger.h"
#include "includes/shared/errors/error_manager.h"
#include "includes/shared/errors/error_models.h"
#include "includes/shared/utils/string_utils.hpp"

#include <algorithm>
#include <unordered_map>
#include <unordered_set>

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

    ErrorManager& errorManager = ErrorManager::GetInstance();
    errorManager.Clear();
    errorManager.SetDeferredMode(true);

    // Step 1: Parse all source files (sequential)
    std::vector<std::unique_ptr<ParserLogic>> parsers;
    std::vector<const AST*> asts;

    for (const auto& filePath : this->m_options.inputFilesPath)
    {
        std::filesystem::path fsPath(filePath);
        ErrorManager::SetCurrentFilePath(fsPath.string());

        auto parser = std::make_unique<ParserLogic>(filePath);
        const AST& ast = parser->MainFlow();
        asts.push_back(&ast);
        parsers.push_back(std::move(parser));
    }

    // Step 1.5: Check for circular imports
    bool hasCircularImport = false;
    {
        std::unordered_map<std::string, std::vector<std::string>> importGraph;
        std::unordered_set<std::string> fileStems;
        for (size_t i = 0; i < parsers.size(); i++)
        {
            std::filesystem::path inputPath(this->m_options.inputFilesPath[i]);
            std::string stem = inputPath.stem().string();
            fileStems.insert(stem);
            importGraph[stem] = CollectImports(*asts[i]);
        }
        // DFS to detect cycles
        std::unordered_map<std::string, int> color; // 0=white, 1=gray (in stack), 2=black
        std::vector<std::string> stack;
        std::string cycleStart;
        auto dfs = [&](const std::string& node, auto& self) -> bool {
            if (color[node] == 1) { cycleStart = node; return true; }
            if (color[node] == 2) return false;
            color[node] = 1;
            stack.push_back(node);
            for (const auto& imp : importGraph[node])
            {
                if (fileStems.count(imp) == 0) continue;
                if (self(imp, self)) return true;
            }
            stack.pop_back();
            color[node] = 2;
            return false;
        };
        for (const auto& [stem, _] : importGraph)
        {
            if (color[stem] != 0) continue;
            stack.clear();
            if (dfs(stem, dfs))
            {
                auto it = std::find(stack.begin(), stack.end(), cycleStart);
                std::string cyclePath;
                for (; it != stack.end(); ++it)
                {
                    if (!cyclePath.empty()) cyclePath += " -> ";
                    cyclePath += *it;
                }
                cyclePath += " -> " + cycleStart;
                ErrorManager::SetCurrentFilePath(StringUtils::ToNarrow(this->m_options.inputFilesPath[0]));
                ErrorManager::GetInstance().StoreError(CompileTimeError(
                    ErrorTypes::CIRCULAR_IMPORT,
                    "Circular import detected: " + cyclePath,
                    0, 0,
                    "Remove the circular dependency between these files"
                ));
                hasCircularImport = true;
                break;
            }
        }
    }

    if (hasCircularImport)
    {
        errorManager.SetDeferredMode(false);
        if (errorManager.HasErrorsOccured())
        {
            errorManager.HandleErrors();
        }
        return;
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

    // Step 3: Semantic analysis + code generation (sequential)
    std::vector<std::string> artifactPaths;
    for (size_t i = 0; i < parsers.size(); i++)
    {
        std::filesystem::path inputPath(this->m_options.inputFilesPath[i]);
        std::string perFileName = inputPath.stem().string();

        ErrorManager::SetCurrentFilePath(inputPath.string());

        SemanticAnalysisLogicComponent semantic(*asts[i], projectSymbols);
        const DecoratedAST& decorated = semantic.MainFlow();

        CompilerOptions perFileOptions = this->m_options;
        perFileOptions.outputFileName = std::wstring(perFileName.begin(), perFileName.end());

        CodeGenManager codegen(decorated, perFileOptions);
        artifactPaths.push_back(codegen.MainFlow());
    }

    // Step 4:

    // Step 5: Disable deferred mode, then display all errors at once and abort if any
    errorManager.SetDeferredMode(false);
    if (errorManager.HasErrorsOccured())
    {
        errorManager.HandleErrors();
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

