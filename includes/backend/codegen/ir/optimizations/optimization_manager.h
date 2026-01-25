#pragma once

#include "includes/shared/logger/entity_logger.h"

#include "includes/backend/codegen/ir/optimizations/constant_folding_pass.h"
#include "includes/backend/codegen/ir/optimizations/dce_pass.h"

#include "includes/backend/codegen/ir/ir_templates/ir_module.h"

/**
    * Optimize the IR code based on optimization level (set at compilation request)
*/
class OptimizationManager
{
public:
    OptimizationManager();
    ~OptimizationManager() = default;

    void MainFlow(IRModule& program);

private:
    EntityLogger m_optimizerLogger;
};
