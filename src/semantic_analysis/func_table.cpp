#include "includes/semantic_analysis/func_table.h"

FuncMetaData* FuncTable::FindFunctionByName(const std::string& funcName)
{
    auto function = this->m_functions.find(funcName);
    return function != this->m_functions.end() ? function->second.get() : nullptr;
}

bool FuncTable::CompareFunctionsSignature(
    const FuncMetaData& func1, 
    const FuncMetaData func2
)
{
    if (func1.name != func2.name) return false;
    if (func1.rtype != func2.rtype) return false;
    if (func1.params.size() != func2.params.size()) return false;

    // make sure both signatures params are the same
    for (size_t param = 0; param < func1.params.size(); param++)
    {
        if (func1.params[param].type != func2.params[param].type)
        {
            return false;
        }
    }
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