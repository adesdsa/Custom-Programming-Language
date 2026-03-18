#include "includes/frontend/semantic_analysis/func_table.h"
#include "includes/shared/common/builtin_library.h"

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
    if (this->m_builtinsRegistered) return;
    this->m_builtinsRegistered = true;

    // Register arraylib and string builtins so semantic analysis can resolve them
    for (const std::string& libName : { "arraylib", "string" })
    {
        const BuiltinLibrary* lib = BuiltinLibraryRegistry::GetLibrary(libName);
        if (!lib) continue;
        for (const BuiltinFunction& bf : lib->functions)
        {
            if (this->FindFunctionByName(bf.name)) continue;
            FuncMetaData meta(
                this->m_entityLogger,
                bf.name,
                bf.returnType,
                bf.params,
                0, 0,
                nullptr
            );
            meta.SetDefined();
            this->InsertFunction(std::move(meta));
        }
    }

    this->m_entityLogger.Info("Registered built-in functions");
}