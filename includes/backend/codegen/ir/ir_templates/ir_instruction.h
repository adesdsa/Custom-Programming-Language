#pragma once

#include <vector>

#include "includes/backend/codegen/ir/ir_templates/ir_base_value.h"
#include "includes/shared/common/codegen/ir/ir_common_types.h"

class BasicBlock;
class IRFunction;

/// @brief Represents a single LLVM-like IR instruction.
/// An instruction has an opcode and zero or more operands, also it's parent block is saved.
class IRInstruction : public IRBaseValue 
{
public:
    IRInstruction(const std::string& name,
        std::shared_ptr<const IRType> type,
        IROpcode opcode,
        std::vector<IRBaseValue*> operands,
        BasicBlock* codeBlock);
    
    // <------------Getters------------> //
    IROpcode GetOpcode() const noexcept;
    const std::vector<IRBaseValue*>& GetOperands() const noexcept;
    IRBaseValue* GetOperand(size_t index) const;
    size_t GetNumOperands() const noexcept;
    BasicBlock* GetParent() const noexcept;

    /// @brief All code blocks should end with a teminator that can be
    /// br/CondBr/ret so the code will know where to continue after the block.
    bool IsTerminator() const noexcept;

    bool IsFuncCall() const noexcept;

    void SetCalledFunction(IRFunction* func) noexcept;

    IRFunction* GetCalledFunction() const noexcept;

    std::string ToString() const override;

    void SetParent(BasicBlock* block) noexcept;

    void AddOperand(IRBaseValue* operand) noexcept;

private:
    IROpcode m_opcode;  // operation of the instruction: add, sub, load etc...
    std::vector<IRBaseValue*> m_operands;  // operands of the instruction operation
    BasicBlock* m_parent;  // block of code this instrcution is inside
    IRFunction* m_calledFunc; // incase we are inside a CALL this is the func that is called and the operands are the args
};

