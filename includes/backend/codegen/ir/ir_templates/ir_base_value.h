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
    IRBaseValue(const std::string& name, std::shared_ptr<const IRType> type);

    virtual ~IRBaseValue() = default;

    virtual std::string ToString() const = 0;

    const std::string& GetName() const noexcept;
    std::shared_ptr<const IRType> GetType() const noexcept;

private:
    std::string m_name; // identifier (e.g., "x", "t0")
    std::shared_ptr<const IRType> m_type;
};
