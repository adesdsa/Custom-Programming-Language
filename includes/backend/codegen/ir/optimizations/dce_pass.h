#pragma once

#include <unordered_set>

#include "includes/backend/codegen/ir/optimizations/abstract_optimization_pass.h"
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

    bool Run(IRFunction& function) override;

private:
    const EntityLogger& m_optimizerLogger;

    std::unordered_set<IRInstruction*> m_usedInstructions;

    /// @brief Check whether an instruction has side effect or not
    /// Side effect:
    ///     * Modifies state outside the function's local scope
    ///     * Interacts with the external functions
    ///     * Depends on external state that might change  
    bool IsInstructionHasSideEffects(IRInstruction* instruction) const;

    /// @brief Add given instruction to `m_usedInstructions`, meaning it aint dead unused code
    void MarkInstructionAsUsed(IRInstruction* instruction);
};
