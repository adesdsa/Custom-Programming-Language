#include "includes/backend/codegen/ir/ir_base_value.h"

IRBaseValue::IRBaseValue(const std::string& SName, std::shared_ptr<const IRType> type)
    : m_SName(SName), m_type(type) {}

std::string IRBaseValue::GetName() const noexcept
{
    return this->m_SName;
}

std::shared_ptr<const IRType> IRBaseValue::GetType() const noexcept
{
    return this->m_type;
}
