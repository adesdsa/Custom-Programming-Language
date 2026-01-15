#include "includes/backend/codegen/ir_constant_values/ir_constant_float.h"

IRConstantFloat::IRConstantFloat(int64_t val, std::shared_ptr<IRType> type)
    : IRBaseValue("", type), m_nValue(val) {}

std::string IRConstantFloat::ToString() const override 
{
    return std::to_string(this->m_nValue);
}

double IRConstantFloat::GetValue() const noexcept
{
    return this->m_nValue;
}