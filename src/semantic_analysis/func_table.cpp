#include "includes/semantic_analysis/func_table.h"

FuncMetaData* FuncTable::FindFunctionByName(const std::string& funcName)
{
    auto function = this->m_functions.find(funcName);
    return function != this->m_functions.end() ? function->second.get() : nullptr;
}

bool FuncTable::InsertFunction(FuncMetaData&& funcMetadata)
{
    auto existingFunction = this->FindFunctionByName(funcMetadata.name);
    
    if (existingFunction)
    {
        // prototype exists but definition is not exists yet
        if (
            existingFunction->isDeclared && 
            !existingFunction->isDefined && 
            funcMetadata.isDefined
        )
        {
            *existingFunction = std::move(funcMetadata);
            return true;
        }

        return false;
    }

    this->m_functions[funcMetadata.name] = std::make_unique<FuncMetaData>(
        std::move(funcMetadata)
    );
    return true;
}
