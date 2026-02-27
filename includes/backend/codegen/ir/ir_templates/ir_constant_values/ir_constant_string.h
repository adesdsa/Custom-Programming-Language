#pragma once

#include <string>
#include "includes/backend/codegen/ir/ir_templates/ir_base_value.h"

/// @brief Represents a string literal constant in the IR.
/// The value stored is the raw content without quotes
class IRConstantString : public IRBaseValue 
{
public:
    IRConstantString(const std::string& val, std::shared_ptr<const IRType> type);

    std::string ToString() const override;

    /// @return The string content
    const std::string& GetValue() const noexcept;

    /// @return The label name for this string in the .data section
    /// @example (e.g., "__str0")
    const std::string& GetLabel() const noexcept;

    void SetLabel(const std::string& label) noexcept;

private:
    std::string m_value;
    std::string m_label;
};

