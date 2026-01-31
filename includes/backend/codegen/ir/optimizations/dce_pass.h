#pragma once

#include <unordered_set>
#include <algorithm>

#include "includes/backend/codegen/ir/optimizations/abstract_optimization_pass.h"

#include "includes/backend/codegen/ir/ir_templates/ir_function.h"
#include "includes/backend/codegen/ir/ir_templates/ir_global_vars.h"
#include "includes/backend/codegen/ir/ir_templates/ir_basic_block.h"
#include "includes/backend/codegen/ir/ir_templates/ir_instruction.h"

/// @brief Removes instructions whose results are never used.
/// 
/// Works backwards from roots (instructions with side effects like ret, store, call)
/// and marks everything they depend on as alive. Anything else marked as dead
/// and can be removed
class DCEPass : public IOptimizationPass
{
public:
    DCEPass(const EntityLogger& optimizerLogger);
    ~DCEPass() = default;

    bool Run(IRModule& program) override;

private:
    const EntityLogger& m_optimizerLogger;

    std::unordered_set<IRGlobalVariable*> m_usedGlobals;
    std::unordered_set<BasicBlock*> m_deadBlocks;
    std::unordered_set<IRInstruction*> m_usedInstructions;

    /// @brief Check whether an instruction has side effect or not
    /// Side effect:
    ///     * Modifies state outside the function's local scope
    ///     * Interacts with the external functions
    ///     * Depends on external state that might change  
    bool IsInstructionHasSideEffects(IRInstruction* instruction) const;

    /// @brief Add given instruction to `m_usedInstructions`, meaning it aint dead unused code
    void MarkInstructionAsUsed(IRInstruction* instruction);

    /// @brief Does following DCE optimization:
    ///     * Remove all unused basic blocks
    ///     * Remove all unsed instructions
    /// @param function function to optimize.
    /// @return `bool`: true -> DCE deleted dead code, false -> DCE wasn't needed
    bool OptimizeFunction(IRFunction& function);
};
