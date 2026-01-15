#include "includes/backend/codegen/ir_constant_values/ir_constant_int.h"

IRConstantInt::IRConstantInt(int64_t val, std::shared_ptr<IRType> type)
    : IRBaseValue("", type), m_nValue(val) {}

std::string IRConstantInt::ToString() const override 
{
    return std::to_string(this->m_nValue);
}

int64_t IRConstantInt::GetValue() const noexcept
{
    return this->m_nValue;
}