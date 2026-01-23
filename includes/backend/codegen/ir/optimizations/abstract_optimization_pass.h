#pragma once

#include "includes/shared/logger/entity_logger.h"

#include "includes/backend/codegen/ir/ir_templates/ir_module.h"

class IOptimizationPass
{
public:
    virtual ~IOptimizationPass() = default;

    /// @brief Run the optimization pass technique on the entire moudle.
    /// @param program The moudle to optimize.
    /// @returns `bool` - true -> Changes have been made, false -> Otherwise.
    virtual bool Run(IRModule& program) = 0;
};
