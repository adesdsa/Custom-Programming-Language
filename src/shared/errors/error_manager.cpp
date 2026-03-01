#include "includes/shared/errors/error_manager.h"

ErrorManager::ErrorManager() : m_entityLogger("ERROR-MANAGER") {}

ErrorManager& ErrorManager::GetInstance() noexcept
{
    static ErrorManager instance;  // created on first call
    return instance;
}

void ErrorManager::SetCurrentFilePath(const std::string& filePath) noexcept
{
    this->m_currentFilePath = filePath;
}

void ErrorManager::SetCurrentReporter(const std::string& reporter) noexcept
{
    this->m_currentReporter = reporter;
}

void ErrorManager::StoreError(CompileTimeError error)
{
    if (!this->m_currentFilePath.empty())
    {
        error.SetFilePath(this->m_currentFilePath);
    }
    if (!this->m_currentReporter.empty())
    {
        error.SetReporter(this->m_currentReporter);
    }
    this->m_errorList.push_back(std::move(error));
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
        const std::string& reporter = error.GetReporter();
        if (!reporter.empty() && ENTITY_COLORS.count(reporter))
        {
            // log under the component that reported the error
            EntityLogger reporterLogger(reporter);
            reporterLogger.Error(error.ToString());
        }
        else
        {
            // fallback to ERROR-MANAGER if no source is set
            this->m_entityLogger.Error(error.ToString());
        }
    }
}

void ErrorManager::AbortCompilation() const
{
    throw CompilationAbortException();  // will be caught by main
}