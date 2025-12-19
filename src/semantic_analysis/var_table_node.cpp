#include "includes/semantic_analysis/var_table_node.h"
#include "var_table_node.h"

VarTableNode::VarTableNode(VarTableNode* parent):
    m_parent(parent),
    m_nCurrentStackOffset(0)
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
    this->m_vars.insert({varMetaData.sName, varMetaData});
    return true;
}

VarTableNode* VarTableNode::CreateBlock()
{
    this->m_children.push_back(std::make_unique<VarTableNode>(this));
    return this->m_children.back().get();
}

std::optional<VarMetaData> VarTableNode::FindVar(const std::string& sVarName)
{
    auto findValue = this->m_vars.find(sVarName);
    if (findValue != this->m_vars.end())
    {
        return findValue->second;
    }
    else if (this->m_parent == nullptr) // Incase we are in the root.
    {
        return std::nullopt;
    }
    else // Checking if the var was created at the larger scope.
    {
        return this->m_parent->FindVar(sVarName);
    }
}

int VarTableNode::GetStackOffset()
{
    return this->m_nCurrentStackOffset;
}

VarTableNode* VarTableNode::GetBlock(int nBlockIndex)
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
