#include "includes/backend/codegen/ir/ir_templates/ir_global_vars.h"

IRGlobalVariable::IRGlobalVariable(
    const std::string& name, 
    std::shared_ptr<const IRType> type
) : 
    IRBaseValue(name, type), 
    m_parent(nullptr), 
    m_initializer(nullptr)
{}

IRModule* IRGlobalVariable::GetParent() const noexcept
{
    return this->m_parent;
}

IRBaseValue* IRGlobalVariable::GetInitializer() const
{
    return this->m_initializer;
}

void IRGlobalVariable::SetParent(IRModule* module) noexcept
{
    this->m_parent = module;
}

void IRGlobalVariable::SetInitializer(IRBaseValue* init)
{
    this->m_initializer = init;
}

std::string IRGlobalVariable::ToString() const
{
    return "@" + this->GetName();  // globals use @ prefix
}

std::string IRGlobalVariable::GetInitializerString() const
{
    std::shared_ptr<const IRType> varType = this->m_initializer->GetType();
    if (varType == IRType::GetInt32() ||
        varType == IRType::GetInt64())
    {
        return std::to_string(static_cast<IRConstantInt*>(this->m_initializer)->GetValue());
    }
    else if (varType == IRType::GetFloat32() ||
        varType == IRType::GetFloat64())
    {
        return std::to_string(static_cast<IRConstantFloat*>(this->m_initializer)->GetValue());
    }
    else if (varType == IRType::GetInt1())
    {
        return static_cast<IRConstantBool*>(this->m_initializer)->GetValue() ? "1" : "0";
    }
    else
    {
        return "0"; // error (shouldnt get here)
    }
}
