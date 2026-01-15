#pragma once

#include "includes/backend/codegen/ir/ir_base_value.h"

/// @note: we take upper bound value as double because at the end of the day it can hold all the smaller floats and can be easly converted to them
class IRConstantFloat : public IRBaseValue 
{
public:
    /// @note: we set name to be empty because it just a magic value with no identifier
    IRConstantFloat(int64_t val, std::shared_ptr<IRType> type);
    
    std::string ToString() const override;

    double GetValue() const noexcept;

private:
    double m_nValue;
};
