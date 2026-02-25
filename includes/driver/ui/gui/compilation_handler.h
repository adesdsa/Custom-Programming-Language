#pragma once

#include "includes/driver/compiler_options.h"

#include <QObject>
#include <QString>

/// @brief Handles the compilation pipeline (validation → orchestrator → result).
///        Reports progress and outcomes via Qt signals so the UI stays decoupled.
class CompilationHandler : public QObject
{
    Q_OBJECT

public:
    explicit CompilationHandler(QObject* parent = nullptr);
    ~CompilationHandler() override = default;

    /// @brief Validate the options and run the compiler.
    void Compile(CompilerOptions options);

signals:
    void LogMessage(const QString& message);
    void ValidationFailed(const QString& error);
    void CompilationSucceeded();
    void CompilationFailed(const QString& error);
};

