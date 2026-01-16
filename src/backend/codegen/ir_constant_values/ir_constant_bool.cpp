#include "includes/backend/codegen/ir_constant_values/ir_constant_bool.h"

IRConstantBool::IRConstantBool(bool bVal) : 
    IRBaseValue("", IRType::GetInt1()), 
    m_bValue(bVal) {}

std::string IRConstantBool::ToString() const
{
    return this->m_bValue ? "true" : "false";
}

bool IRConstantBool::GetBool() const noexcept
{
    return this->m_bValue;
}
