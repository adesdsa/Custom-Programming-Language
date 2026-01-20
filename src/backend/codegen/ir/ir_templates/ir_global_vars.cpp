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
