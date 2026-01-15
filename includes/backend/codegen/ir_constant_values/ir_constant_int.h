#pragma once

#include "includes/backend/codegen/ir/ir_base_value.h"

/// @note: we take upper bound value as int64_t because at the end of the day it can hold all the smaller ints and can be easily converted to them
class IRConstantInt : public IRBaseValue 
{
public:
    /// @note: we set name to be empty because it just a magic value with no identifier
    IRConstantInt(int64_t val, std::shared_ptr<IRType> type);
    
    std::string ToString() const override;

    int64_t GetValue() const noexcept;

private:
    int64_t m_nValue;
};
