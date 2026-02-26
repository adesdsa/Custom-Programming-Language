#include "includes/shared/errors/error_manager.h"

ErrorManager::ErrorManager() : m_entityLogger("ERROR-MANAGER") {}

ErrorManager& ErrorManager::GetInstance() noexcept
{
    static ErrorManager instance;  // created on first call
    return instance;
}

void ErrorManager::StoreError(const CompileTimeError& error)
{
    this->m_errorList.push_back(error);
}

void ErrorManager::Clear() noexcept
{
    this->m_errorList.clear();
}

bool ErrorManager::HasErrorsOccured() const noexcept
{
    return this->m_errorList.size() > 0;
}

void ErrorManager::HandleErrors() const
{
    if (!this->HasErrorsOccured()) return;
    
    this->DisplayErrorsInUI();
    this->AbortCompilation();
}

void ErrorManager::DisplayErrorsInUI() const noexcept
{
    if (this->m_errorList.empty()) return;

    for (const CompileTimeError& error : this->m_errorList)
    {
        this->m_entityLogger.Error(error.ToString());
    }
}

void ErrorManager::AbortCompilation() const
{
    throw CompilationAbortException();  // will be caught by main
}