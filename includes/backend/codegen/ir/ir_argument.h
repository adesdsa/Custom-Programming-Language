#pragma once

#include "ir_base_value.h"

class IRFunction;  // forward declaration

/// @brief The class is used to represent an intermidiate language function arguement.
/// Lets say we have the next function declaration:
/// define i32 @max(i32 %a, i32 %b)  
/// {
///                     ^^     ^^
///                     │      └── IRArgument: 
///                     │         name="b", type=Int32, index=1
///                     │         parent = max 
///                     │
///                     └── IRArgument: 
///                           name="a",type=Int32 index=0,
///                           parent = max
/// }
class IRArgument : public IRBaseValue 
{
public:
    IRArgument(const std::string& name,
        std::shared_ptr<const IRType> type,
        size_t index,
        IRFunction* func = nullptr
    );

    // Getters.
    size_t GetIndex() const noexcept;
    IRFunction* GetParent() const noexcept;

    std::string ToString() const override;

private:
    size_t m_argIndex;  // order of the param in the function params start from 0
    IRFunction* m_parent;   // function that owns this argument
};

