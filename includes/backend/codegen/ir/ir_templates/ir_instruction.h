#pragma once

#include <vector>

#include "includes/backend/codegen/ir/ir_templates/ir_base_value.h"
#include "includes/shared/common/codegen/ir/ir_common_types.h"

class BasicBlock;
class IRFunction;

struct PhiIncoming
{
    IRBaseValue* value;
    BasicBlock* from;
};

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
    
    // ===== Getters =====
    IROpcode GetOpcode() const noexcept;
    const std::vector<IRBaseValue*>& GetOperands() const noexcept;
    IRBaseValue* GetOperand(size_t index) const;
    size_t GetNumOperands() const noexcept;
    BasicBlock* GetParent() const noexcept;
    /// @brief All code blocks should end with a teminator that can be
    /// br/CondBr/ret so the code will know where to continue after the block.
    bool IsTerminator() const noexcept;
    bool IsFuncCall() const noexcept;
    IRFunction* GetCalledFunction() const noexcept;

    // ===== Setters =====
    void SetCalledFunction(IRFunction* func) noexcept;
    void SetParent(BasicBlock* block) noexcept;
    void AddOperand(IRBaseValue* operand) noexcept;

    // ===== Phi-specific API =====
    bool IsPhi() const noexcept;
    void AddIncoming(IRBaseValue* value, BasicBlock* from);
    size_t GetNumIncoming() const noexcept;
    const PhiIncoming& GetIncoming(size_t index) const;
    const std::vector<PhiIncoming>& GetIncomingList() const noexcept;

    std::string ToString() const override;

private:
    IROpcode m_opcode;  // operation of the instruction: add, sub, load etc...
    std::vector<IRBaseValue*> m_operands;  // operands of the instruction operation
    std::vector<PhiIncoming> m_incoming;     // ONLY used for Phi
    BasicBlock* m_parent;  // block of code this instrcution is inside
    IRFunction* m_calledFunc; // incase we are inside a CALL this is the func that is called and the operands are the args
};

