#pragma once

#include <string>
#include <memory>

#include "includes/backend/codegen/ir/ir_type.h"

/// @brief This is an abstract interface class that is designed to hold a single variable.
/// All variables have same kind name and type(IRType) and a different value thus an 
/// abstract interface is right in this case.
class IRBaseValue 
{
public:
    IRBaseValue(const std::string& SName, std::shared_ptr<const IRType> type);

    virtual ~IRBaseValue() = default;

    virtual std::string ToString() const = 0;

    virtual std::string GetName() const noexcept;

    virtual std::shared_ptr<const IRType> GetType() const noexcept;

private:
    std::string m_SName; // identifier (e.g., "x", "t0")
    std::shared_ptr<const IRType> m_type;
};

