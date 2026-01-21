#include "includes/backend/codegen/ir/ir_templates/ir_basic_block.h"

BasicBlock::BasicBlock(const std::string& label) :
    m_label(label),
    m_parent(nullptr),
    m_terminator(nullptr)
{}

const std::string& BasicBlock::GetLabel() const noexcept
{
    return this->m_label;
}

IRFunction* BasicBlock::GetParent() const noexcept
{
    return this->m_parent;
}

const std::list<std::unique_ptr<IRInstruction>>& BasicBlock::GetInstructions() const noexcept
{
    return this->m_instructions;
}

IRInstruction* BasicBlock::GetTerminator() const noexcept
{
    return this->m_terminator;
}

bool BasicBlock::HasTerminator() const noexcept
{
    return this->m_terminator != nullptr;
}

const std::vector<BasicBlock*>& BasicBlock::GetPredecessors() const noexcept
{
    return this->m_predecessors;
}

const std::vector<BasicBlock*>& BasicBlock::GetSuccessors() const noexcept
{
    return this->m_successors;
}

void BasicBlock::SetParent(IRFunction* func) noexcept
{
    this->m_parent = func;
}

void BasicBlock::AddPredecessor(BasicBlock* block)
{
    this->m_predecessors.push_back(block);
}

void BasicBlock::AddSuccessor(BasicBlock* block)
{
    this->m_successors.push_back(block);
}

void BasicBlock::AddInstruction(std::unique_ptr<IRInstruction> instruction)
{
    if (this->m_terminator) return; // After endpoint no more adding.

    instruction->SetParent(this);
    if (instruction->IsTerminator())
    {
        this->m_terminator = instruction.get();
    }

    this->m_instructions.push_back(std::move(instruction));
}

void BasicBlock::AddFirstInstruction(std::unique_ptr<IRInstruction> instruction)
{
    if (this->m_terminator) return; // After endpoint no more adding.

    instruction->SetParent(this);
    if (instruction->IsTerminator())
    {
        this->m_terminator = instruction.get();
    }

    this->m_instructions.insert(this->m_instructions.begin(), std::move(instruction));
}
