#pragma once

#include <string>
#include <memory>

#include "includes/backend/codegen/ir/ir_type.h"

class IRBaseValue 
{
public:
    IRBaseValue(const std::string& name, std::shared_ptr<const IRType> type);

    virtual ~IRBaseValue() = default;

    virtual std::string ToString() const = 0;

    virtual std::string GetName() const noexcept;

    virtual std::shared_ptr<const IRType> GetType() const noexcept;

private:
    std::string m_sName; // identifier (e.g., "x", "t0")
    std::shared_ptr<const IRType> m_type;
};

