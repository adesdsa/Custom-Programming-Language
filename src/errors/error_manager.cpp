#include "errors/error_manager.h"

ErrorManager::ErrorManager() : m_entityLogger("ERROR-MANAGER") {}

ErrorManager& ErrorManager::GetInstance() noexcept
{
    static ErrorManager instance;  // created on first call
    return instance;
}

void ErrorManager::StoreError(const CompileTimeError &error)
{
    this->m_errorList.push_back(error);
}

void ErrorManager::DisplayErrorsInUI()
{
    for(const CompileTimeError& error : this->m_errorList)
    {
        this->m_entityLogger.Error(error.ToString());
    }
}