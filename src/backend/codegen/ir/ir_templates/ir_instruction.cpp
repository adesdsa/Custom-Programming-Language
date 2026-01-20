#include "includes/backend/codegen/ir/ir_templates/ir_instruction.h"

IRInstruction::IRInstruction(
    const std::string& name,
    std::shared_ptr<const IRType> type,
    IROpcode opcode,
    std::vector<IRBaseValue*> operands,
    BasicBlock* codeBlock
) :
    IRBaseValue(name, type),
    m_opcode(opcode),
    m_operands(std::move(operands)),
    m_parent(codeBlock)
{}

IROpcode IRInstruction::GetOpcode() const noexcept
{
    return this->m_opcode;
}

const std::vector<IRBaseValue*>& IRInstruction::GetOperands() const noexcept
{
    return this->m_operands;
}

IRBaseValue* IRInstruction::GetOperand(size_t index) const
{
    if (this->m_operands.size() <= index) return nullptr;
    return this->m_operands[index];
}

size_t IRInstruction::GetNumOperands() const noexcept
{
    return this->m_operands.size();
}

BasicBlock* IRInstruction::GetParent() const noexcept
{
    return this->m_parent;
}

// Is this Br (jump, CondBr (jump with condition, or Ret (return of a function)
// every BasicBlock must end with a terminator instruction, which controls where execution goes next
bool IRInstruction::IsTerminator() const noexcept
{
    return (
        this->m_opcode == IROpcode::Br     || 
        this->m_opcode == IROpcode::CondBr || 
        this->m_opcode == IROpcode::Ret
    );
}

std::string IRInstruction::ToString() const
{
    // some instructions do not have a name
    if (this->GetName().empty()) return "";

    return "%" + this->GetName();
}

void IRInstruction::SetParent(BasicBlock* block) noexcept
{
    this->m_parent = block;
}

void IRInstruction::AddOperand(IRBaseValue* operand) noexcept
{
    this->m_operands.push_back(operand);
}
