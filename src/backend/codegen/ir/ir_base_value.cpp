#include "includes/backend/codegen/ir/ir_base_value.h"

IRBaseValue::IRBaseValue(const std::string& sName, std::shared_ptr<IRType> type)
    : m_sName(sName), m_type(type) {}

std::string IRBaseValue::GetName() const noexcept
{
    return this->m_sName;
}

std::shared_ptr<IRType> IRBaseValue::GetType() const noexcept
{
    return this->m_type;
}
