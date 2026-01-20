#include "includes/backend/codegen/ir/ir_templates/ir_base_value.h"

IRBaseValue::IRBaseValue(const std::string& name, std::shared_ptr<const IRType> type) : 
    m_name(name), 
    m_type(type)
{}

const std::string& IRBaseValue::GetName() const noexcept
{
    return this->m_name;
}

std::shared_ptr<const IRType> IRBaseValue::GetType() const noexcept
{
    return this->m_type;
}
