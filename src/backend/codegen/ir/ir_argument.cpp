#include "includes/backend/codegen/ir/ir_argument.h"

IRArgument::IRArgument(const std::string& SName,
    std::shared_ptr<IRType> type,
    size_t UIndex,
    IRFunction* func) :
    IRBaseValue(SName, type),
    m_UArgIndex(UIndex),
    m_parent(func) {}

size_t IRArgument::GetIndex() const noexcept 
{ 
    return this->m_UArgIndex;
}

IRFunction* IRArgument::GetParent() const noexcept 
{ 
    return this->m_parent;
}

std::string IRArgument::ToString() const
{
    return "%" + this->GetName();
}