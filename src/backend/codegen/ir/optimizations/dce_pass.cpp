#include "includes/backend/codegen/ir/optimizations/dce_pass.h"

DCEPass::DCEPass(const EntityLogger& optimizerLogger) :
    m_optimizerLogger(optimizerLogger)
{}

bool DCEPass::Run(IRFunction& function)
{
    this->m_optimizerLogger.Info("Run Dead Code Elimination optimization");
    bool hasChanged = false;
    this->m_usedInstructions.clear();

    // mark all instructions with side effects as used
    for (const auto& basicBlock : function.GetBasicBlocks())
    {
        for (const auto& instruction : basicBlock->GetInstructions())
        {
            if (this->IsInstructionHasSideEffects(instruction.get()))
            {
                this->MarkInstructionAsUsed(instruction.get());
            }
        }
    }

    // remove dead instructions
    std::vector<IRInstruction*> instructionsToRemove;

    for (const auto& basicBlock : function.GetBasicBlocks())
    {
        for (const auto& instruction : basicBlock->GetInstructions())
        {
            if (
                this->m_usedInstructions.find(
                    instruction.get()
                ) == this->m_usedInstructions.end()
            )
            {
                instructionsToRemove.push_back(instruction.get());
            }
        }
    }

    for (IRInstruction* instruction : instructionsToRemove)
    {
        this->m_optimizerLogger.Warnning("Removing unused action: " + instruction->ToString());
        instruction->GetParent()->RemoveInstruction(instruction);
        hasChanged = true;
    }

    return hasChanged;
}

bool DCEPass::IsInstructionHasSideEffects(IRInstruction* instruction) const
{
    if (!instruction) return false;

    IROpcode opcode = instruction->GetOpcode();

    switch (opcode)
    {
        case IROpcode::Ret:
        case IROpcode::Br:
        case IROpcode::CondBr:
        case IROpcode::Store:
        case IROpcode::Call:
            return true;

        default:
            return false;
    }
}

void DCEPass::MarkInstructionAsUsed(IRInstruction* instruction)
{
    if (!instruction) return;

    // already marked
    if (this->m_usedInstructions.find(instruction) != this->m_usedInstructions.end())
    {
        return;
    }

    this->m_usedInstructions.insert(instruction);

    // mark all operands (which can be instruction without side effect) as used too
    for (IRBaseValue* operand : instruction->GetOperands())
    {
        if (auto* operandInstruction = dynamic_cast<IRInstruction*>(operand))
        {
            this->MarkInstructionAsUsed(operandInstruction);
        }
    }

    // for Phi nodes, mark incoming values
    if (instruction->IsPhi())
    {
        for (const auto& incoming : instruction->GetIncomingList())
        {
            if (auto* opInst = dynamic_cast<IRInstruction*>(incoming.value))
            {
                this->MarkInstructionAsUsed(opInst);
            }
        }
    }
}
