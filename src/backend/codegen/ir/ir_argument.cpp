#include "includes/backend/codegen/ir/ir_argument.h"

IRArgument::IRArgument(const std::string& sName,
    std::shared_ptr<IRType> type,
    size_t uIndex,
    IRFunction* func) :
    IRBaseValue(sName, type),
    m_uArgIndex(uIndex),
    m_parent(func) {}

size_t IRArgument::GetIndex() const noexcept 
{ 
    return this->m_uArgIndex;
}

IRFunction* IRArgument::GetParent() const noexcept 
{ 
    return this->m_parent;
}

std::string IRArgument::ToString() const
{
    return "%" + this->GetName();
}