#include "includes/semantic_analysis/func_table.h"

FuncMetaData* FuncTable::FindFunctionByName(const std::string& funcName) const
{
    auto function = this->m_functions.find(funcName);
    return function != this->m_functions.end() ? function->second.get() : nullptr;
}

bool FuncTable::InsertFunction(FuncMetaData&& funcMetadata)
{
    auto existingFunction = this->FindFunctionByName(funcMetadata.GetName());
    
    if (existingFunction)
    {
        // prototype exists but definition is not exists yet
        if (
            existingFunction->IsFuncDeclared() && 
            !existingFunction->IsFuncDefined() && 
            funcMetadata.IsFuncDefined()
        )
        {
            *existingFunction = std::move(funcMetadata);
            return true;
        }

        return false;
    }

    this->m_functions[funcMetadata.GetName()] = std::make_unique<FuncMetaData>(
        std::move(funcMetadata)
    );
    return true;
}
