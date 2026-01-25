#include "includes/backend/codegen/ir/optimizations/optimization_manager.h"

OptimizationManager::OptimizationManager() :
    m_optimizerLogger("OPTIMIZATION")
{
    this->m_optimizerLogger.Info("Optimizer component initalized successfully");
}

void OptimizationManager::MainFlow(IRModule &program)
{
    {
        DCEPass dcePass(this->m_optimizerLogger);
        dcePass.Run(program);
    }

    {
        ConstantFoldingPass constantFoldingPass(this->m_optimizerLogger);
        constantFoldingPass.Run(program);
    }
}
