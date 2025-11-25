#pragma once

#include <vector>

#include "error_models.h"

// Will handle all the compile time errors. Store them and then handle
// them one by one and at the end abort compiling process
class ErrorManager
{  
public:
    // Delete copy/move to prevent duplicates
    ErrorManager(const Console&) = delete;
    ErrorManager& operator=(const Console&) = delete;

    // Create the only instance this class will have throughout the program
    // @return: Static instance of the console class.
    static ErrorManager& GetInstance() noexcept;

    // Stores the given error for later handling
    void StoreError(const CompileTimeError& error);

    // TODO: For later versions - Make this method support both GUI & CLI
    // Display all the errors that occur through out the compilation in the UI
    void DisplayErrorsInUI();

private:
    std::vector<CompileTimeError> m_errorList;

    // Safely stops the compilation proccess
    void AbortCompilation();
};
