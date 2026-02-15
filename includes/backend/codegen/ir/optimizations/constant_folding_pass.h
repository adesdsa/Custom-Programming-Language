#pragma once

#include "includes/backend/codegen/ir/optimizations/abstract_optimization_pass.h"

#include "includes/backend/codegen/ir/ir_templates/ir_constant_values/ir_constant_int.h"
#include "includes/backend/codegen/ir/ir_templates/ir_constant_values/ir_constant_float.h"
#include "includes/backend/codegen/ir/ir_templates/ir_constant_values/ir_constant_bool.h"
#include "includes/backend/codegen/ir/ir_templates/ir_function.h"
#include "includes/backend/codegen/ir/ir_templates/ir_instruction.h"

constexpr size_t NUMBER_OF_OPERANDS_IN_BINARY_OPERATION = 2;
constexpr size_t LEFT_OPERAND = 0;
constexpr size_t RIGHT_OPERAND = 1;

/// @brief Evaluates expressions with constant operands at compile-time rather than runtime.
/// 
/// @example  add i32 3, 5 -> Evaluates as 8
class ConstantFoldingPass : public IOptimizationPass
{
public:
    ConstantFoldingPass(const EntityLogger& optimizerLogger);
    ~ConstantFoldingPass() = default;

    bool Run(IRModule& program) override;

private:
    const EntityLogger& m_optimizerLogger;

    std::vector<std::unique_ptr<IRBaseValue>> m_foldedConstants;

    /// @brief Run constant folding optimization on given function.
    /// @param function function to optimize.
    /// @return `bool`: true -> Function has been optimizied, false -> Otherwise. 
    bool OptimizeFunction(IRFunction& function);

    /// @brief Check if fold is possible on given instruction, is so evaluate it.
    /// @param instruction instruction to fold.
    /// @return IF: Foldable -> the evaluation of the instruction, ELSE: nullptr
    IRBaseValue* TryFoldInstruction(IRInstruction* instruction);

    /// @brief Replace all occurrences of old values before folded into folded version.
    /// @param function Function in which the instruction is at.
    /// @param oldValue Value at the instruction before folding.
    /// @param newValue Value at the instruction after folding.
    void ReplaceAllUses(
        IRFunction& function, 
        IRBaseValue* oldValue, 
        IRBaseValue* newValue
    );

    bool IsFoldable(const IROpcode& opcode);

    std::unique_ptr<IRConstantInt> TryEvaluateIntegerBinaryOp(
        IROpcode opcode, 
        int64_t lhs, 
        int64_t rhs,
        std::shared_ptr<const IRType> resultType
    );

    std::unique_ptr<IRConstantFloat> TryEvaluateFloatBinaryOp(
        IROpcode opcode, 
        double lhs, 
        double rhs,
        std::shared_ptr<const IRType> resultType
    );

    std::unique_ptr<IRConstantBool> TryEvaluateIntegerComparison(
        IROpcode opcode, 
        int64_t lhs, 
        int64_t rhs
    );

    std::unique_ptr<IRConstantBool> TryEvaluateFloatComparison(
        IROpcode opcode, 
        double lhs, 
        double rhs
    );
};
