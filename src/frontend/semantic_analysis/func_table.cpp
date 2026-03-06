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
        this->m_entityLogger.Warn(
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

void FuncTable::RegisterBuiltins()
{
    // guard: only register once
    if (this->FindFunctionByName("println")) return;

    // println(value: int) -> void
    std::vector<VarMetaData> printlnParams = {
        VarMetaData("value", Type::INT_TYPE, AccessRights::DEFAULT_ACCESS, Scope::LET, 0)
    };
    FuncMetaData printlnFunc(
        this->m_entityLogger,
        "println",
        Type::VOID_TYPE,
        printlnParams,
        0, 0,  // no source location (built-in)
        nullptr // no scope root (built-in)
    );
    printlnFunc.SetDefined(); // mark as defined so the linker check passes
    this->InsertFunction(std::move(printlnFunc));

    // str_concat(a: string, b: string) -> string
    std::vector<VarMetaData> strConcatParams = {
        VarMetaData("a", Type::STRING_TYPE, AccessRights::DEFAULT_ACCESS, Scope::LET, 0),
        VarMetaData("b", Type::STRING_TYPE, AccessRights::DEFAULT_ACCESS, Scope::LET, 0)
    };
    FuncMetaData strConcatFunc(
        this->m_entityLogger,
        "str_concat",
        Type::STRING_TYPE,
        strConcatParams,
        0, 0,
        nullptr
    );
    strConcatFunc.SetDefined();
    this->InsertFunction(std::move(strConcatFunc));

    this->m_entityLogger.Info("Registered built-in functions");
}