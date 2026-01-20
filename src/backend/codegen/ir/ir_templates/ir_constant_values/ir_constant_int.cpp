#include "includes/backend/codegen/ir_templates/ir_constant_values/ir_constant_int.h"

IRConstantInt::IRConstantInt(int64_t val, std::shared_ptr<const IRType> type) : 
    IRBaseValue("", type), 
    m_value(val) 
{}

std::string IRConstantInt::ToString() const
{
    return std::to_string(this->m_value);
}

int64_t IRConstantInt::GetValue() const noexcept
{
    return this->m_value;
}
