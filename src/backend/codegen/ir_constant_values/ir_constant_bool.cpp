#include "includes/backend/codegen/ir_constant_values/ir_constant_bool.h"

IRConstantBool::IRConstantBool(bool BVal) : 
    IRBaseValue("", IRType::GetInt1()), 
    m_BValue(BVal) {}

std::string IRConstantBool::ToString() const
{
    return this->m_BValue ? "true" : "false";
}

bool IRConstantBool::GetBool() const noexcept
{
    return this->m_BValue;
}