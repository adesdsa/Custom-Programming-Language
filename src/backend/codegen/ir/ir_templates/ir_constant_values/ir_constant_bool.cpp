#include "includes/backend/codegen/ir_constant_values/ir_constant_bool.h"

IRConstantBool::IRConstantBool(bool val) : 
    IRBaseValue("", IRType::GetInt1()), 
    m_value(val) 
{}

std::string IRConstantBool::ToString() const
{
    return this->m_value ? "true" : "false";
}

bool IRConstantBool::GetBool() const noexcept
{
    return this->m_value;
}
