#include "includes/frontend/semantic_analysis/func_table.h"

FuncMetaData* FuncTable::FindFunctionByName(const std::string& funcName) const
{
    auto function = this->m_functions.find(funcName);
    return function != this->m_functions.end() ? function->second.get() : nullptr;
}

bool FuncTable::InsertFunction(FuncMetaData&& funcMetadata)
{
    auto existingFunction = this->FindFunctionByName(funcMetadata.GetName());
    // We use move and only then GetName, by this point the metada is empty so we need to save the name.
    std::string sFuncName = funcMetadata.GetName();
    
    if (existingFunction)
    {
        if (
            existingFunction->IsFuncDeclared() && 
            !existingFunction->IsFuncDefined() && 
            funcMetadata.IsFuncDefined()
        )
        {
            this->m_entityLogger.Info("Insert function declaration to an existing function definition");
            this->m_functions[sFuncName] =
                std::make_unique<FuncMetaData>(std::move(funcMetadata));
            return true;
        }
        this->m_entityLogger.Warnning(
            "Function " + sFuncName + " already exists. Inseration failed"
        );
        return false;
    }

    this->m_entityLogger.Info("Inserting a new function into function table");
    this->m_functions[sFuncName] = std::make_unique<FuncMetaData>(
        std::move(funcMetadata)
    );
    return true;
}

const std::unordered_map<std::string, std::unique_ptr<FuncMetaData>>&
FuncTable::GetAllFunctions() const noexcept
{
    return this->m_functions;
}