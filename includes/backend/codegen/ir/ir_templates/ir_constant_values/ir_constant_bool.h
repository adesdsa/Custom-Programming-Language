#pragma once

#include "includes/backend/codegen/ir/ir_templates/ir_base_value.h"

/// @brief A class that represents a magic boolean value.
class IRConstantBool : public IRBaseValue
{
public:
    /// @note: we set name to be empty because it just a magic value with no identifier
    IRConstantBool(bool val);
    
    std::string ToString() const override;

    bool GetBool() const noexcept;

private:
    bool m_value;
};
