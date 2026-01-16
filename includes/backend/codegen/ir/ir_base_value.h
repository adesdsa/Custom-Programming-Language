#pragma once

#include <string>
#include <memory>

#include "includes/backend/codegen/ir/ir_type.h"

/// @brief This is an abstract interface class that holds certain metadata
/// that any llvm IR object that has a value stores.
/// Any llvm IR object that can hold a value should inherit this.
class IRBaseValue 
{
public:
    IRBaseValue(const std::string& sName, std::shared_ptr<const IRType> type);

    virtual ~IRBaseValue() = default;

    virtual std::string ToString() const = 0;

    virtual std::string GetName() const noexcept;

    virtual std::shared_ptr<const IRType> GetType() const noexcept;

private:
    std::string m_sName; // identifier (e.g., "x", "t0")
    std::shared_ptr<const IRType> m_type;
};

