#include "includes/backend/codegen/ir/ir_templates/ir_constant_values/ir_constant_string.h"

IRConstantString::IRConstantString(const std::string& val, std::shared_ptr<const IRType> type)
    : IRBaseValue("", type), m_value(val), m_label("") {}

std::string IRConstantString::ToString() const
{
    return "\"" + this->m_value + "\"";
}

const std::string& IRConstantString::GetValue() const noexcept
{
    return this->m_value;
}

const std::string& IRConstantString::GetLabel() const noexcept
{
    return this->m_label;
}

void IRConstantString::SetLabel(const std::string& label) noexcept
{
    this->m_label = label;
}

