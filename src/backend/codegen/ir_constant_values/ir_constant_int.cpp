#include "includes/backend/codegen/ir_constant_values/ir_constant_int.h"

IRConstantInt::IRConstantInt(int64_t nVal, std::shared_ptr<const IRType> type)
    : IRBaseValue("", type), m_nValue(nVal) {}

std::string IRConstantInt::ToString() const
{
    return std::to_string(this->m_nValue);
}

int64_t IRConstantInt::GetValue() const noexcept
{
    return this->m_nValue;
}
