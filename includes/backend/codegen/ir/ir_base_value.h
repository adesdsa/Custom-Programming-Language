#pragma once

#include <string>
#include <memory>

#include "includes/backend/codegen/ir/ir_type.h"

class IRBaseValue 
{
public:
    std::string m_sName; // identifier (e.g., "x", "t0")
    std::shared_ptr<IRType> m_type;

    IRBaseValue(const std::string& name, std::shared_ptr<IRType> type);

    virtual ~IRBaseValue() = default;

    virtual std::string ToString() const = 0;
};

