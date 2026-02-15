#include "includes/backend/codegen/ir/ir_templates/ir_function.h"

IRFunction::IRFunction(
    const std::string& name,
    std::shared_ptr<const IRType> functionType
) :
    m_name(name),
    m_functionType(functionType),
    m_parent(nullptr)
{}

const std::string& IRFunction::GetName() const noexcept
{
    return this->m_name;
}

std::shared_ptr<const IRType> IRFunction::GetFunctionType() noexcept
{
    return this->m_functionType;
}

std::shared_ptr<const IRType> IRFunction::GetReturnType() const noexcept
{
    return this->m_functionType->GetReturnType();
}

const std::vector<std::shared_ptr<const IRType>>& IRFunction::GetArguemntsTypes() const noexcept
{
    return this->m_functionType->GetParameters();
}

IRModule* IRFunction::GetParent() const noexcept
{
    return this->m_parent;
}

IRArgument* IRFunction::GetArgument(size_t index) const
{
    if (this->m_arguments.size() <= index) return nullptr;
    else return this->m_arguments[index].get();
}

IRArgument* IRFunction::GetArgument(const std::string& name) const
{
    auto result = this->m_argumentMap.find(name);
    return (result != this->m_argumentMap.end()) ? result->second : nullptr;
}

const std::vector<std::unique_ptr<IRArgument>>& IRFunction::GetArguments() const noexcept
{
    return this->m_arguments;
}

size_t IRFunction::GetNumArguments() const noexcept
{
    return this->m_arguments.size();
}

BasicBlock* IRFunction::GetEntryBlock() const
{
    if (this->m_basicBlocks.empty()) return nullptr;
    else return this->m_basicBlocks[ENTRY_BLOCK_INDEX].get();
}

BasicBlock* IRFunction::GetBasicBlock(const std::string& label) const
{
    auto result = this->m_blockMap.find(label);
    return (result != this->m_blockMap.end()) ? result->second : nullptr;
}

const std::vector<std::unique_ptr<BasicBlock>>& IRFunction::GetBasicBlocks() const noexcept
{
    return this->m_basicBlocks;
}

size_t IRFunction::GetNumBasicBlocks() const noexcept
{
    return this->m_basicBlocks.size();
}

IRArgument* IRFunction::AddArgument(const std::string& name, std::shared_ptr<const IRType> type)
{
    if (this->m_argumentMap.find(name) != this->m_argumentMap.end()) return nullptr;

    this->m_arguments.push_back(std::make_unique<IRArgument>(
        name,
        type,
        this->m_arguments.size(),
        this
    ));
    this->m_argumentMap[name] = this->m_arguments.back().get();
    
    return this->m_argumentMap[name];
}

void IRFunction::SetParent(IRModule* module) noexcept
{
    this->m_parent = module;
}

BasicBlock* IRFunction::AddBasicBlock(const std::string& label)
{
    if (this->m_blockMap.find(label) != this->m_blockMap.end()) return nullptr;

    this->m_basicBlocks.push_back(std::make_unique<BasicBlock>(label));
    this->m_blockMap[label] = this->m_basicBlocks.back().get();
    this->m_blockMap[label]->SetParent(this);
    
    return this->m_blockMap[label];
}

BasicBlock* IRFunction::AddBasicBlock(std::unique_ptr<BasicBlock> copyBlock)
{
    // save label before moving the unique_ptr
    std::string label = copyBlock->GetLabel();
    
    if (this->m_blockMap.find(label) != this->m_blockMap.end()) return nullptr;

    this->m_basicBlocks.push_back(std::move(copyBlock));
    this->m_blockMap[label] = this->m_basicBlocks.back().get();
    this->m_blockMap[label]->SetParent(this);
    
    return this->m_blockMap[label];
}

IRInstruction* IRFunction::FindAllocatedVar(const std::string& name)
{
    if (this->m_basicBlocks.size() == 0) return nullptr;
    for (auto& instruction : this->GetEntryBlock()->GetInstructions())
    {
        if (instruction.get()->GetName() == name) return instruction.get();
    }
    return nullptr;
}

int IRFunction::CountTempVars() const
{
    int cntTemps = 0;
    for (const auto& block : this->m_basicBlocks)
    {
        cntTemps += block->CountTempVars();
    }
    return cntTemps;
}

void IRFunction::RemoveBasicBlock(BasicBlock *basicBlock)
{
    if (!basicBlock) return;

    this->m_blockMap.erase(basicBlock->GetLabel());

    // find basic block in vector
    auto basicBlockIterator = std::find_if(
        this->m_basicBlocks.begin(),
        this->m_basicBlocks.end(),
        [basicBlock](const std::unique_ptr<BasicBlock>& ptr) 
        {
            return ptr.get() == basicBlock;
        }
    );

    if (basicBlockIterator != this->m_basicBlocks.end())
    {
        this->m_basicBlocks.erase(basicBlockIterator);
    }
}
