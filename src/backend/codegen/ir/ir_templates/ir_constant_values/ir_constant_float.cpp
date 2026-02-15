#include "includes/backend/codegen/ir/ir_templates/ir_constant_values/ir_constant_float.h"

IRConstantFloat::IRConstantFloat(double val, std::shared_ptr<const IRType> type) : 
    IRBaseValue("", type), 
    m_value(val) 
{}

std::string IRConstantFloat::ToString() const
{
    return std::to_string(this->m_value);
}

double IRConstantFloat::GetValue() const noexcept
{
    return this->m_value;
}
