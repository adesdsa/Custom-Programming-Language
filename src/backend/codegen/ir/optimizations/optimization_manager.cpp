#include "includes/backend/codegen/ir/optimizations/optimization_manager.h"

OptimizationManager::OptimizationManager() :
    m_optimizerLogger("OPTIMIZATION")
{
    this->m_optimizerLogger.Info("Optimizer component initalized successfully");
}

// TODO: For better complexity, try BFS/DFS or use-def chain
void OptimizationManager::MainFlow(IRModule& program)
{
    DCEPass dcePass(this->m_optimizerLogger);
    ConstantFoldingPass constantFoldingPass(this->m_optimizerLogger);
    
    bool changed = true;

    while (changed)
    {
        changed = false;
        
        if (dcePass.Run(program))
        {
            changed = true;
        }
        
        if (constantFoldingPass.Run(program))
        {
            changed = true;
        }
    }

    this->m_optimizerLogger.Info("Optimizer done.");
}
