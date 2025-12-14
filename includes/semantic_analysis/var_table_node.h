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

    // The func inserts the variable into the block if it doesnt exist.
    // @param varMetaData: The meta data of the added var.
    // @return: True -> The insertion worked. False -> The insertion failed.
    bool InsertVar(const VarMetaData& varMetaData);

    // The func creates a new block for more vars.
    // @return: Pointer to the new block.
    VarTableNode* CreateBlock();

    // The func searches for a variable in the current scope.
    // @param sVarName: The var we are looking for.
    // @return: The meta data of the var if it exists.
    std::optional<VarMetaData> FindVar(const std::string& sVarName);

    // The func return the stackOffset.
    int GetStackOffset();

    // The func returns the block at the given index.
    // @return: nullpt -> The block doesnt exist. Otherwise -> The block at the given index.
    VarTableNode* GetBlock(int nBlockIndex);

private:
    std::unordered_map<std::string, VarMetaData> m_vars;
    VarTableNode* m_parent;
    std::vector<std::unique_ptr<VarTableNode>> m_children;
    int m_nCurrentStackOffset;

};
