#pragma once

#include "includes/shared/logger/entity_logger.h"
#include "includes/backend/codegen/ir/ir_templates/ir_function.h"

class IOptimizationPass
{
public:
    virtual ~IOptimizationPass() = default;

    /// @brief Run the optimization pass on a function.
    /// @param function The function to optimize.
    /// @returns `bool` - true -> Changes have been made, false -> Otherwise.
    virtual bool Run(IRFunction& function) = 0;
};
