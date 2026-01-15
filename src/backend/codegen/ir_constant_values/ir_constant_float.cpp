#include "includes/backend/codegen/ir_constant_values/ir_constant_float.h"

IRConstantFloat::IRConstantFloat(int64_t NVal, std::shared_ptr<const IRType> type)
    : IRBaseValue("", type), m_FValue(NVal) {}

std::string IRConstantFloat::ToString() const
{
    return std::to_string(this->m_FValue);
}

double IRConstantFloat::GetValue() const noexcept
{
    return this->m_FValue;
}