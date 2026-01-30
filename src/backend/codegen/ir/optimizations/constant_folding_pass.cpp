#include "includes/backend/codegen/ir/optimizations/constant_folding_pass.h"

ConstantFoldingPass::ConstantFoldingPass(const EntityLogger &optimizerLogger) :
    m_optimizerLogger(optimizerLogger)
{}

bool ConstantFoldingPass::Run(IRModule& program)
{
    this->m_optimizerLogger.Info("Running Constant Folding on module: " + program.GetName());
    bool hasChanged = false;

    for (const auto& function : program.GetFunctions())
    {
        if (this->OptimizeFunction(*function))
        {
            hasChanged = true;
        }
    }

    return hasChanged;
}

bool ConstantFoldingPass::OptimizeFunction(IRFunction& function)
{
    bool hasChanged = false;
    bool madeProgress = true;

    // keep iterating until no more folding possible
    while (madeProgress)
    {
        madeProgress = false;
        std::vector<IRInstruction*> instructionsToRemove;

        for (const auto& basicBlock : function.GetBasicBlocks())
        {
            for (const auto& instruction : basicBlock->GetInstructions())
            {
                IRBaseValue* folded = this->TryFoldInstruction(instruction.get());

                if (folded)
                {
                    this->m_optimizerLogger.Info(
                        "Folded: " + instruction->ToString() + " -> " + folded->ToString()
                    );

                    // replace all uses of this instruction with the constant
                    this->ReplaceAllUses(function, instruction.get(), folded);

                    instructionsToRemove.push_back(instruction.get());
                    madeProgress = true;
                    hasChanged = true;
                }
            }
        }

        for (IRInstruction* instruction : instructionsToRemove)
        {
            instruction->GetParent()->RemoveInstruction(instruction);
        }
    }

    return hasChanged;
}

IRBaseValue* ConstantFoldingPass::TryFoldInstruction(IRInstruction* instruction)
{
    if (!instruction) return nullptr;

    IROpcode opcode = instruction->GetOpcode();
    if (!this->IsFoldable(opcode)) return nullptr;

    const auto& operands = instruction->GetOperands();

    // binary operations have exactly 2 operands
    if (operands.size() != NUMBER_OF_OPERANDS_IN_BINARY_OPERATION)
    {
        return nullptr;
    }

    IRBaseValue* lhsVal = operands[LEFT_OPERAND];
    IRBaseValue* rhsVal = operands[RIGHT_OPERAND];

    // =================================
    // | Try Folding Integer Constants |
    // =================================
    auto* lhsInt = dynamic_cast<IRConstantInt*>(lhsVal);
    auto* rhsInt = dynamic_cast<IRConstantInt*>(rhsVal);

    if (lhsInt && rhsInt)
    {
        int64_t lhs = lhsInt->GetValue();
        int64_t rhs = rhsInt->GetValue();

        if (
            auto result = this->TryEvaluateIntegerBinaryOp(
                opcode, lhs, rhs, instruction->GetType()
            )
        )
        {
            this->m_foldedConstants.push_back(std::move(result));
            return this->m_foldedConstants.back().get();
        }

        if (auto result = this->TryEvaluateIntegerComparison(opcode, lhs, rhs))
        {
            this->m_foldedConstants.push_back(std::move(result));
            return this->m_foldedConstants.back().get();
        }
    }

    // ===============================
    // | Try Folding Float Constants |
    // ===============================
    auto* lhsFloat = dynamic_cast<IRConstantFloat*>(lhsVal);
    auto* rhsFloat = dynamic_cast<IRConstantFloat*>(rhsVal);

    if (lhsFloat && rhsFloat)
    {
        double lhs = lhsFloat->GetValue();
        double rhs = rhsFloat->GetValue();

        if (
            auto result = this->TryEvaluateFloatBinaryOp(
                opcode, lhs, rhs, instruction->GetType()
            )
        )
        {
            this->m_foldedConstants.push_back(std::move(result));
            return this->m_foldedConstants.back().get();
        }

        if (auto result = this->TryEvaluateFloatComparison(opcode, lhs, rhs))
        {
            this->m_foldedConstants.push_back(std::move(result));
            return this->m_foldedConstants.back().get();
        }
    }

    return nullptr;
}

void ConstantFoldingPass::ReplaceAllUses(
    IRFunction& function, 
    IRBaseValue* oldValue, 
    IRBaseValue* newValue
)
{
    for (const auto& basicBlock : function.GetBasicBlocks())
    {
        for (const auto& instruction : basicBlock->GetInstructions())
        {
            // replace old operands with the folded version
            const auto& operands = instruction->GetOperands();
            for (size_t i = 0; i < operands.size(); i++)
            {
                if (operands[i] == oldValue)
                {
                    instruction->SetOperand(i, newValue);
                }
            }

            // replace in Phi incoming values
            if (instruction->IsPhi())
            {
                for (size_t i = 0; i < instruction->GetNumIncoming(); i++)
                {
                    const auto& incoming = instruction->GetIncoming(i);
                    if (incoming.value == oldValue)
                    {
                        instruction->SetIncomingValue(i, newValue);
                    }
                }
            }
        }
    }
}

bool ConstantFoldingPass::IsFoldable(const IROpcode& opcode)
{
    switch (opcode)
    {
        // non foldable instructions
        case IROpcode::Alloca:
        case IROpcode::Load:
        case IROpcode::Store:
        case IROpcode::Call:
        case IROpcode::Br:
        case IROpcode::CondBr:
        case IROpcode::Ret:
        case IROpcode::Phi:
            return true;

        default:
            return false;
    }
}

std::unique_ptr<IRConstantInt> ConstantFoldingPass::TryEvaluateIntegerBinaryOp(
    IROpcode opcode, 
    int64_t lhs, 
    int64_t rhs,
    std::shared_ptr<const IRType> resultType
)
{
    int64_t result = 0;

    switch (opcode)
    {
        case IROpcode::Add:
            result = lhs + rhs;
            break;

        case IROpcode::Sub:
            result = lhs - rhs;
            break;

        case IROpcode::Mul:
            result = lhs * rhs;
            break;

        case IROpcode::SDiv:
            if (rhs == 0) return nullptr;  // division by zero
            result = lhs / rhs;
            break;

        case IROpcode::UDiv:
            if (rhs == 0) return nullptr;
            result = static_cast<int64_t>(
                static_cast<uint64_t>(lhs) / static_cast<uint64_t>(rhs)
            );
            break;

        case IROpcode::SRem:
            if (rhs == 0) return nullptr;
            result = lhs % rhs;
            break;

        case IROpcode::URem:
            if (rhs == 0) return nullptr;
            result = static_cast<int64_t>(
                static_cast<uint64_t>(lhs) % static_cast<uint64_t>(rhs)
            );
            break;

        case IROpcode::And:
            result = lhs & rhs;
            break;

        case IROpcode::Or:
            result = lhs | rhs;
            break;

        case IROpcode::Xor:
            result = lhs ^ rhs;
            break;

        case IROpcode::Shl:
            result = lhs << rhs;
            break;

        case IROpcode::AShr:
            result = lhs >> rhs;
            break;

        case IROpcode::LShr:
            result = static_cast<int64_t>(
                static_cast<uint64_t>(lhs) >> static_cast<uint64_t>(rhs)
            );
            break;

        default:
            return nullptr;
    }

    return std::make_unique<IRConstantInt>(result, resultType);
}

std::unique_ptr<IRConstantFloat> ConstantFoldingPass::TryEvaluateFloatBinaryOp(
    IROpcode opcode, 
    double lhs, 
    double rhs,
    std::shared_ptr<const IRType> resultType
)
{
    double result;

    switch (opcode)
    {
        case IROpcode::FAdd:
            result = lhs + rhs;
            break;

        case IROpcode::FSub:
            result = lhs - rhs;
            break;

        case IROpcode::FMul:
            result = lhs * rhs;
            break;

        case IROpcode::FDiv:
            if (rhs == 0.0) return nullptr;  // division by zero
            result = lhs / rhs;
            break;

        default:
            return nullptr;
    }

    return std::make_unique<IRConstantFloat>(result, resultType);
}

std::unique_ptr<IRConstantBool> ConstantFoldingPass::TryEvaluateIntegerComparison(
    IROpcode opcode, 
    int64_t lhs, 
    int64_t rhs
)
{
    bool result;

    switch (opcode)
    {
        case IROpcode::ICmpEq:
            result = (lhs == rhs);
            break;

        case IROpcode::ICmpNe:
            result = (lhs != rhs);
            break;

        case IROpcode::ICmpSlt:
            result = (lhs < rhs);
            break;

        case IROpcode::ICmpSle:
            result = (lhs <= rhs);
            break;

        case IROpcode::ICmpSgt:
            result = (lhs > rhs);
            break;

        case IROpcode::ICmpSge:
            result = (lhs >= rhs);
            break;

        case IROpcode::ICmpUlt:
            result = (static_cast<uint64_t>(lhs) < static_cast<uint64_t>(rhs));
            break;

        case IROpcode::ICmpUle:
            result = (static_cast<uint64_t>(lhs) <= static_cast<uint64_t>(rhs));
            break;

        case IROpcode::ICmpUgt:
            result = (static_cast<uint64_t>(lhs) > static_cast<uint64_t>(rhs));
            break;

        case IROpcode::ICmpUge:
            result = (static_cast<uint64_t>(lhs) >= static_cast<uint64_t>(rhs));
            break;

        default:
            return nullptr;
    }

    return std::make_unique<IRConstantBool>(result);
}

std::unique_ptr<IRConstantBool> ConstantFoldingPass::TryEvaluateFloatComparison(
    IROpcode opcode, 
    double lhs, 
    double rhs
)
{
    bool result;

    switch (opcode)
    {
        case IROpcode::FCmpEq:
            result = (lhs == rhs);
            break;

        case IROpcode::FCmpNe:
            result = (lhs != rhs);
            break;

        case IROpcode::FCmpLt:
            result = (lhs < rhs);
            break;

        case IROpcode::FCmpLe:
            result = (lhs <= rhs);
            break;

        case IROpcode::FCmpGt:
            result = (lhs > rhs);
            break;

        case IROpcode::FCmpGe:
            result = (lhs >= rhs);
            break;

        default:
            return nullptr;
    }

    return std::make_unique<IRConstantBool>(result);
}
