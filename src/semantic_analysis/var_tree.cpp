#include "includes/semantic_analysis/var_tree.h"

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
        // todo: add error that var exists
        return true;
    }

    // todo: handle the currentStackOffset
    this->m_vars.insert({varMetaData.sName, varMetaData});
    return false;
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
    else if (this->m_parent == nullptr)
    {
        return std::nullopt;
    }
    else
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
