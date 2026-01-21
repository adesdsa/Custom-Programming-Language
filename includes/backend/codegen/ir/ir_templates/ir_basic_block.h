#pragma once

#include <list>
#include <string>
#include <vector>
#include <memory>

#include "includes/backend/codegen/ir/ir_templates/ir_instruction.h"

class IRFunction;

/// @brief Represents a single basic block in a function's control-flow graph (CFG).
///
/// A BasicBlock contains a sequence of IR instructions in SSA form and
/// terminates with a control-flow instruction (e.g., `ret` or `br`).  
/// It also tracks its position in the CFG via predecessor and successor blocks.
///
/// - Owns its instructions (`std::unique_ptr<IRInstruction>`).  
/// - Maintains a raw pointer to the terminator for quick access.  
/// - Predecessors are blocks that can jump to this block.  
/// - Successors are blocks that this block can jump to.  
/// - Belongs to an `IRFunction`, accessible via `GetParent()`.

class BasicBlock 
{
public:
    BasicBlock(const std::string& label);
    
    const std::string& GetLabel() const noexcept;
    IRFunction* GetParent() const noexcept;
    const std::list<std::unique_ptr<IRInstruction>>& GetInstructions() const noexcept;
    IRInstruction* GetTerminator() const noexcept;
    bool HasTerminator() const noexcept;

    
    // ===== CFG NAVIGATION =====
    const std::vector<BasicBlock*>& GetPredecessors() const noexcept;
    const std::vector<BasicBlock*>& GetSuccessors() const noexcept;
    
    // ===== Setters =====
    void SetParent(IRFunction* func) noexcept;
    void AddPredecessor(BasicBlock* block);
    void AddSuccessor(BasicBlock* block);
    void AddInstruction(std::unique_ptr<IRInstruction> instruction);
    void AddFirstInstruction(std::unique_ptr<IRInstruction> instruction);

private:
    const std::string m_label;  // block name                                   
    IRFunction* m_parent;  // owner of the block                                  

    std::list<std::unique_ptr<IRInstruction>>m_instructions;  // the instruction (code) inside the block
    // This saves the pointer to the instruction that is owned in the list above.
    IRInstruction* m_terminator;  // ret, br etc...
    
    std::vector<BasicBlock*> m_predecessors;  // blocks that jump to here
    std::vector<BasicBlock*> m_successors;    // blocks we jump to
};
