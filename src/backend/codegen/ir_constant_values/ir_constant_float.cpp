#include "includes/backend/codegen/ir_constant_values/ir_constant_float.h"

IRConstantFloat::IRConstantFloat(double fVal, std::shared_ptr<const IRType> type)
    : IRBaseValue("", type), m_fValue(fVal) {}

std::string IRConstantFloat::ToString() const
{
    return std::to_string(this->m_fValue);
}

double IRConstantFloat::GetValue() const noexcept
{
    return this->m_fValue;
}
