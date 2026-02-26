#pragma once

#include <vector>

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

    // Stores the given error for later handling
    void StoreError(const CompileTimeError& error);

    // Remove all previously stored errors (call at the start of a new compilation)
    void Clear() noexcept;

    // Display all errors that occured and abort program
    void HandleErrors() const;

    // @return: true -> Errors stored in the `m_errorList`, false -> Otherwise.
    bool HasErrorsOccured() const noexcept;

private:
    std::vector<CompileTimeError> m_errorList;
    EntityLogger m_entityLogger;

    // TODO: For later versions - Make this method support both GUI & CLI
    // Display all the errors that occur through out the compilation in the UI
    void DisplayErrorsInUI() const noexcept;

    // Safely stops the compilation proccess
    void AbortCompilation() const;
};
