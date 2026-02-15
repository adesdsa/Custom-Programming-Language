#include "includes/frontend/semantic_analysis/var_table_node.h"

VarTableNode::VarTableNode(VarTableNode* parent, const EntityLogger& entityLogger):
    m_parent(parent),
    m_nCurrentStackOffset(0),
    m_entityLogger(entityLogger)
{
}

bool VarTableNode::InsertVar(const VarMetaData& varMetaData)
{
    auto foundVar = FindVar(varMetaData.sName);
    if (foundVar)
    {
        return false;
    }

    this->m_nCurrentStackOffset -= varMetaData.offsetInsideFrame;
    
    VarMetaData adjustedVar = varMetaData;
    adjustedVar.offsetInsideFrame = this->m_nCurrentStackOffset;
    this->m_vars.insert({adjustedVar.sName, adjustedVar});
    m_entityLogger.Info("Added a new var to the varTable");
    return true;
}

VarTableNode* VarTableNode::CreateBlock()
{
    m_entityLogger.Info("Created a new block inside the varTable");
    auto child = std::make_unique<VarTableNode>(this, m_entityLogger);
    child->m_nCurrentStackOffset = this->m_nCurrentStackOffset;
    this->m_children.push_back(std::move(child));
    return this->m_children.back().get();
}

VarTableNode* VarTableNode::GetParent() const noexcept
{
    return this->m_parent;
}

std::optional<VarMetaData> VarTableNode::FindVar(const std::string& sVarName) const
{
    auto findValue = this->m_vars.find(sVarName);
    if (findValue != this->m_vars.end())
    {
        return findValue->second;
    }
    else if (this->m_parent == nullptr)
    {
        return std::nullopt;
    }
    else
    {
        return this->m_parent->FindVar(sVarName);
    }
}

int VarTableNode::GetStackOffset() const
{
    return this->m_nCurrentStackOffset;
}

VarTableNode* VarTableNode::GetBlock(int nBlockIndex) const
{
    if (nBlockIndex >= this->m_children.size())
    {
        return nullptr;
    }
    return this->m_children[nBlockIndex].get();
}

const std::unordered_map<std::string, VarMetaData> &VarTableNode::GetVars() const noexcept
{
    return this->m_vars;
}

const std::vector<std::unique_ptr<VarTableNode>> &VarTableNode::GetChildren() const noexcept
{
    return this->m_children;
}

VarTableNode& VarTableNode::operator[](size_t index)
{
    return *m_children.at(index);
}

const VarTableNode& VarTableNode::operator[](size_t index) const
{
    return *m_children.at(index);
}
