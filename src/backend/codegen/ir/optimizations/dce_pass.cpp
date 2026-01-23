#include "includes/backend/codegen/ir/optimizations/dce_pass.h"

DCEPass::DCEPass(const EntityLogger& optimizerLogger) :
    m_optimizerLogger(optimizerLogger)
{}

bool DCEPass::Run(IRModule& program)
{
    this->m_optimizerLogger.Info("Running DCE on module: " + program.GetName());

    bool hasChanged = false;
    this->m_usedGlobals.clear();

    for (const auto& function : program.GetFunctions())
    {
        if (this->OptimizeFunction(*function))
        {
            hasChanged = true;
        }
    }

    std::vector<IRGlobalVariable*> globalsToRemove;

    for (const auto& global : program.GetVariables())
    {
        if (this->m_usedGlobals.count(global.get()) == 0)
        {
            globalsToRemove.push_back(global.get());
        }
    }

    // remove all unused global variables
    for (IRGlobalVariable* global : globalsToRemove)
    {
        m_optimizerLogger.Warnning("Removing unused global: " + global->GetName());
        program.RemoveVariable(global);
        hasChanged = true;
    }

    return hasChanged;
}

bool DCEPass::OptimizeFunction(IRFunction& function)
{
    this->m_optimizerLogger.Info("Optimizing function: " + function.GetName());
    bool hasChanged = false;

    this->m_usedInstructions.clear();
    this->m_deadBlocks.clear();

    std::vector<BasicBlock*> basicBlocksToRemove;
    std::vector<IRInstruction*> instructionsToRemove;

    for (const auto& basicBlock : function.GetBasicBlocks())
    {
        // block consider dead if it not an entry block AND no one jumping to it
        bool isDeadBlock = (
                basicBlock.get() != function.GetEntryBlock() && 
                basicBlock->GetPredecessors().empty()
            );

        if (isDeadBlock)
        {
            basicBlocksToRemove.push_back(basicBlock.get());
            this->m_deadBlocks.insert(basicBlock.get());
            continue;  // no need to keep checking the instruction of the block irrelvant
        }

        for (const auto& instruction : basicBlock->GetInstructions())
        {
            if (this->IsInstructionHasSideEffects(instruction.get()))
            {
                this->MarkInstructionAsUsed(instruction.get());
            }
        }
    }

    // put all unused instructions in the removal list
    for (const auto& basicBlock : function.GetBasicBlocks())
    {
        if (this->m_deadBlocks.count(basicBlock.get()) > 0)
        {
            continue;  // no need to keep checking the instruction of the block irrelvant
        }

        for (const auto& instruction : basicBlock->GetInstructions())
        {
            if (this->m_usedInstructions.count(instruction.get()) == 0)
            {
                instructionsToRemove.push_back(instruction.get());
            }
        }
    }

    for (BasicBlock* basicBlock : basicBlocksToRemove)
    {
        this->m_optimizerLogger.Warnning(
            "Removing unreachable block: " + basicBlock->GetLabel()
        );
        function.RemoveBasicBlock(basicBlock);
        hasChanged = true;
    }

    for (IRInstruction* instruction : instructionsToRemove)
    {
        this->m_optimizerLogger.Warnning(
            "Removing dead instruction: " + instruction->ToString()
        );
        instruction->GetParent()->RemoveInstruction(instruction);
        hasChanged = true;
    }

    return hasChanged;
}

bool DCEPass::IsInstructionHasSideEffects(IRInstruction* instruction) const
{
    if (!instruction) return false;

    switch (instruction->GetOpcode())
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
    // if instruction not exist or already marked
    if (!instruction || this->m_usedInstructions.count(instruction) > 0) 
    {
        return;
    }

    this->m_usedInstructions.insert(instruction);

    // mark operands as used and collect global variable references
    for (IRBaseValue* operand : instruction->GetOperands())
    {
        // check if operand is a global variable
        if (auto* global = dynamic_cast<IRGlobalVariable*>(operand))
        {
            this->m_usedGlobals.insert(global);
        }
        // check if operand is an instruction
        else if (auto* operandInstruction = dynamic_cast<IRInstruction*>(operand))
        {
            this->MarkInstructionAsUsed(operandInstruction);
        }
    }

    if (instruction->IsPhi())
    {
        for (const auto& incoming : instruction->GetIncomingList())
        {
            if (auto* global = dynamic_cast<IRGlobalVariable*>(incoming.value))
            {
                this->m_usedGlobals.insert(global);
            }
            else if (auto* opInst = dynamic_cast<IRInstruction*>(incoming.value))
            {
                this->MarkInstructionAsUsed(opInst);
            }
        }
    }
}
