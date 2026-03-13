#pragma once

#include <vector>
#include <mutex>

#include "error_models.h"

#include "includes/shared/logger/entity_logger.h"

// Will handle all the compile time errors. Store them and then handle
// them one by one and at the end abort compiling process
class ErrorManager
{  
public:
    ErrorManager();
    ~ErrorManager() = default;

    // Delete copy/move to prevent duplicates
    ErrorManager(const ErrorManager&) = delete;
    ErrorManager& operator=(const ErrorManager&) = delete;

    // Create the only instance this class will have throughout the program
    // @return: Static instance of the console class.
    static ErrorManager& GetInstance() noexcept;

    // Sets the current file path to stamp on all subsequent errors (thread-safe per-thread)
    static void SetCurrentFilePath(const std::string& filePath) noexcept;

    // Sets the current reporter component name to stamp on all subsequent errors (thread-safe per-thread)
    static void SetCurrentReporter(const std::string& reporter) noexcept;

    // Stores the given error for later handling
    void StoreError(CompileTimeError error);

    // Remove all previously stored errors (call at the start of a new compilation)
    void Clear() noexcept;

    // Display all errors that occured and abort program.
    // In deferred mode this is a no-op — errors accumulate silently.
    void HandleErrors() const;

    // @return: true -> Errors stored in the `m_errorList`, false -> Otherwise.
    bool HasErrorsOccured() const noexcept;

    /// @return: true if any stored error is for the given file path.
    bool HasErrorsForFile(const std::string& filePath) const noexcept;

    /// @brief Enable/disable deferred mode.
    /// When deferred, HandleErrors() is a no-op so that all files can be
    /// processed before errors are displayed. Call with false and then
    /// HandleErrors() to display everything at the end.
    void SetDeferredMode(bool deferred) noexcept;

private:
    std::vector<CompileTimeError> m_errorList;
    EntityLogger m_entityLogger;
    mutable std::mutex m_errorManagerMutex;
    bool m_deferredMode = false;

    static thread_local std::string tl_currentFilePath;
    static thread_local std::string tl_currentReporter;

    // Internal helpers that assume m_errorManagerMutex is already held
    bool HasErrorsOccured_NoLock() const noexcept;

    void DisplayErrorsInUI_NoLock() const noexcept;

    // Safely stops the compilation proccess
    void AbortCompilation_NoLock() const;
};
