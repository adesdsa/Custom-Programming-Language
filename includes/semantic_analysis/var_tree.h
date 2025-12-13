#pragma once
#include <optional>
#include <vector>
#include <unordered_map>
#include <memory>
#include "var_table.h"

class VarTableNode
{
public:
    VarTableNode(VarTableNode* parent);
    ~VarTableNode() = default;

    bool InsertVar(const VarMetaData& varMetaData);
    VarTableNode* CreateBlock();
    std::optional<VarMetaData> FindVar(const std::string& sVarName);

    int GetStackOffset();
    VarTableNode* GetBlock(int nBlockIndex);

private:
    std::unordered_map<std::string, VarMetaData> m_vars;
    VarTableNode* m_parent;
    std::vector<std::unique_ptr<VarTableNode>> m_children;
    int m_nCurrentStackOffset;

};
