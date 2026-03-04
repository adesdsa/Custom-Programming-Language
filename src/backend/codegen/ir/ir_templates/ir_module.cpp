#include "includes/backend/codegen/ir/ir_templates/ir_module.h"
#include "includes/backend/codegen/ir/ir_templates/ir_argument.h"

IRModule::IRModule(const std::string& moduleName) :
    m_name(moduleName)
{}

const std::string& IRModule::GetName() const noexcept
{
    return this->m_name;
}

IRFunction* IRModule::GetFunction(const std::string& name) const
{
    auto result = this->m_functionMap.find(name);
    return result != this->m_functionMap.end() ? result->second : nullptr;
}

const std::vector<std::unique_ptr<IRFunction>>& IRModule::GetFunctions() const noexcept
{
    return this->m_functions;
}

size_t IRModule::GetNumFunctions() const noexcept
{
    return this->m_functions.size();
}

IRFunction* IRModule::AddFunction(const std::string& name, std::shared_ptr<const IRType> returnType)
{
    if (this->m_functionMap.find(name) != this->m_functionMap.end()) return nullptr;

    this->m_functions.push_back(std::make_unique<IRFunction>(
        name,
        returnType
    ));
    this->m_functionMap[name] = this->m_functions.back().get();
    this->m_functionMap[name]->SetParent(this);
    
    return this->m_functionMap[name];
}

bool IRModule::IsFuncExistByName(const std::string& name) const
{
    return this->m_functionMap.find(name) != this->m_functionMap.end();
}

IRGlobalVariable* IRModule::GetVariable(const std::string& name) const
{
    auto result = this->m_globalMap.find(name);
    return result != this->m_globalMap.end() ? result->second : nullptr;
}

const std::vector<std::unique_ptr<IRGlobalVariable>>& IRModule::GetVariables() const noexcept
{
    return this->m_globals;
}

size_t IRModule::GetNumVariables() const noexcept
{
    return this->m_globals.size();
}

IRGlobalVariable* IRModule::AddVariable(const std::string& name, std::shared_ptr<const IRType> type, bool isExtern)
{
    if (this->m_globalMap.find(name) != this->m_globalMap.end()) return nullptr;

    this->m_globals.push_back(std::make_unique<IRGlobalVariable>(
        name,
        type,
        isExtern
    ));
    this->m_globalMap[name] = this->m_globals.back().get();
    this->m_globalMap[name]->SetParent(this);
    
    return this->m_globalMap[name];
}

bool IRModule::IsVarExistByName(const std::string& name) const
{
    return this->m_globalMap.find(name) != this->m_globalMap.end();
}

void IRModule::RemoveVariable(IRGlobalVariable* globalVar)
{
    if (!globalVar) return;

    this->m_globalMap.erase(globalVar->GetName());

    // find var in vector
    auto globalVarIterator = std::find_if(
        this->m_globals.begin(),
        this->m_globals.end(),
        [globalVar](const std::unique_ptr<IRGlobalVariable>& ptr) 
        {
            return ptr.get() == globalVar;
        }
    );

    if (globalVarIterator != this->m_globals.end())
    {
        this->m_globals.erase(globalVarIterator);
    }
}

void IRModule::AddConstant(std::unique_ptr<IRBaseValue> constant)
{
    this->m_constants.push_back(std::move(constant));
}

const std::vector<std::unique_ptr<IRBaseValue>>& IRModule::GetConstants() const noexcept
{
    return this->m_constants;
}

const std::vector<std::string>& IRModule::GetExports() const
{
    return this->m_exportedSymbols;
}

const std::vector<std::string>& IRModule::GetImports() const
{
    return this->m_externSymbols;
}

void IRModule::AddExport(const std::string& exportable)
{
    this->m_exportedSymbols.push_back(exportable);
}

void IRModule::AddImport(const std::string& importable)
{
    this->m_externSymbols.push_back(importable);
}

void IRModule::SetPrimaryModule()
{
    this->m_isPrimaryModule = true;
}

bool IRModule::IsPrimaryModule() const
{
    return this->m_isPrimaryModule;
}
