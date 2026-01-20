#include "includes/backend/codegen/ir/ir_templates/ir_function.h"
#include "includes/backend/codegen/ir/ir_templates/ir_argument.h"

IRFunction::IRFunction(
    const std::string& name,
    std::shared_ptr<const IRType> returnType
) :
    m_name(name),
    m_returnType(returnType),
    m_parent(nullptr)
{}

const std::string& IRFunction::GetName() const noexcept
{
    return this->m_name;
}

std::shared_ptr<const IRType> IRFunction::GetReturnType() const noexcept
{
    return this->m_returnType;
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
