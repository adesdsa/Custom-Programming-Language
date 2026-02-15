#pragma once

#include "includes/backend/codegen/ir/ir_templates/ir_base_value.h"

/// @brief A class that represents a magic float number.
class IRConstantFloat : public IRBaseValue 
{
public:
    /// @note: we take upper bound value as double because at the end of the day it can 
    /// hold all the smaller floats and can be easly converted to them.
    /// @note: we set name to be empty because it just a magic value with no identifier
    IRConstantFloat(double val, std::shared_ptr<const IRType> type);
    
    std::string ToString() const override;

    double GetValue() const noexcept;

private:
    double m_value;
};
