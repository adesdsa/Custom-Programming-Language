#include "includes/driver/ui/gui/compilation_handler.h"
#include "includes/driver/options_validator.h"
#include "includes/compiler_orchestrator.h"
#include "includes/shared/logger/console.h"

#include <stdexcept>
#include <filesystem>

CompilationHandler::CompilationHandler(QObject* parent)
    : QObject(parent)
{}

void CompilationHandler::Compile(CompilerOptions options)
{
    emit LogMessage("========================================");
    emit LogMessage("Starting compilation...");
    emit LogMessage("========================================");

    // ---- Validate ----
    try
    {
        OptionsValidator::Validate(options);
    }
    catch (const std::runtime_error& e)
    {
        emit LogMessage("Validation FAILED: " + QString::fromStdString(e.what()));
        emit ValidationFailed(QString::fromStdString(e.what()));
        return;
    }

    // ---- Log the resolved options ----
    emit LogMessage("Input files:");
    for (const auto& file : options.inputFilesPath)
    {
        emit LogMessage("  " + QString::fromStdWString(file));
    }

    std::filesystem::path outputBase = options.outputDir.empty()
        ? std::filesystem::current_path()
        : std::filesystem::path(options.outputDir);
    std::filesystem::path outputFullPath = outputBase / options.outputFileName;
    emit LogMessage("Output: " + QString::fromStdWString(outputFullPath.wstring()));
    emit LogMessage("Optimization: -O" + QString::number(options.optimizationLevel));

    // ---- Register log callback so Console logs appear in the GUI ----
    Console& console = Console::GetInstance();
    console.SetLogCallback([this](const std::string& line)
    {
        emit LogMessage(QString::fromStdString(line));
    });

    // ---- Compile ----
    try
    {
        CompilerOrchestrator orchestrator(std::move(options));
        orchestrator.MainFlow();

        emit LogMessage("========================================");
        emit LogMessage("Compilation SUCCEEDED");
        emit LogMessage("========================================");
        emit CompilationSucceeded();
    }
    catch (const std::exception& e)
    {
        emit LogMessage("========================================");
        emit LogMessage("Compilation FAILED: " + QString::fromStdString(e.what()));
        emit LogMessage("========================================");
        emit CompilationFailed(QString::fromStdString(e.what()));
    }
    catch (...)
    {
        emit LogMessage("Compilation FAILED: Unknown error");
        emit CompilationFailed("An unknown error occurred.");
    }

    // ---- cleanup ----
    console.DisableFileLogging();
    console.SetLogCallback(nullptr);
}
