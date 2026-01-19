#include "includes/backend/codegen/ir/ir_argument.h"

IRArgument::IRArgument(const std::string& name,
    std::shared_ptr<const IRType> type,
    size_t index,
    IRFunction* func) :
    IRBaseValue(name, type),
    m_argIndex(index),
    m_parent(func){}

size_t IRArgument::GetIndex() const noexcept 
{ 
    return this->m_argIndex;
}

IRFunction* IRArgument::GetParent() const noexcept 
{ 
    return this->m_parent;
}

std::string IRArgument::ToString() const
{
    return "%" + this->GetName();
}