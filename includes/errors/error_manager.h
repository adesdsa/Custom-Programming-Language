#pragma once

#include <vector>

#include "error_models.h"

// Will handle all the compile time errors. Store them and then handle
// them one by one and at the end abort compiling process
class ErrorManager
{  
public:
    ErrorManager() = default;
    ~ErrorManager() = default;

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
