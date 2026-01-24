#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>

#include "includes/backend/codegen/ir/ir_templates/ir_basic_block.h"
#include "includes/backend/codegen/ir/ir_templates/ir_argument.h"
#include "includes/backend/codegen/ir/ir_templates/ir_type.h"

constexpr int ENTRY_BLOCK_INDEX = 0;

class IRModule;

/// @brief Represents a single function in an IR module.
///
/// IRFunction owns its arguments and basic blocks, and tracks its parent module.
/// 
/// - Arguments are stored in `m_arguments` (ownership) and `m_argumentMap` (fast lookup by name).  
/// - Basic blocks are stored in `m_basicBlocks` (ownership) and `m_blockMap` (fast lookup by label).  
/// - The first basic block added is typically the entry block.  
/// - Each block and argument knows its parent function.  
/// - Assumes argument and block names are unique within the function.
class IRFunction 
{
public:
    IRFunction(
        const std::string& name,
        std::shared_ptr<const IRType> returnType
    );

    // <======= Getters =======>
    const std::string& GetName() const noexcept;
    std::shared_ptr<const IRType> GetReturnType() const noexcept;
    IRModule* GetParent() const noexcept;

    // <======= Arguement Getters =======>
    IRArgument* GetArgument(size_t index) const;
    IRArgument* GetArgument(const std::string& name) const;
    const std::vector<std::unique_ptr<IRArgument>>& GetArguments() const noexcept;
    /// @return m_arguments size
    size_t GetNumArguments() const noexcept;

    // <======= Block Getters =======>
    BasicBlock* GetEntryBlock() const;
    BasicBlock* GetBasicBlock(const std::string& label) const;
    const std::vector<std::unique_ptr<BasicBlock>>& GetBasicBlocks() const noexcept;
    /// @return m_basicBlocks size
    size_t GetNumBasicBlocks() const noexcept;

    IRArgument* AddArgument(const std::string& name, std::shared_ptr<const IRType> type);
    void SetParent(IRModule* module) noexcept;

    BasicBlock* AddBasicBlock(const std::string& label);
    BasicBlock* AddBasicBlock(std::unique_ptr<BasicBlock> copyBlock);

private:
    const std::string m_name;                                      
    const std::shared_ptr<const IRType> m_returnType;              
    IRModule* m_parent;  // moudle that own the function                                      

    /// @note: Following contains both vector and map version 
    // for best complexity in the job we need to do

    std::vector<std::unique_ptr<IRArgument>> m_arguments;
    std::map<std::string, IRArgument*> m_argumentMap;

    std::vector<std::unique_ptr<BasicBlock>> m_basicBlocks;  
    std::map<std::string, BasicBlock*> m_blockMap;
};
