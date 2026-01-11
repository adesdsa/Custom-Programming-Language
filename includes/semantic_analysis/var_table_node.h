#pragma once

#include <optional>
#include <vector>
#include <unordered_map>
#include <memory>

#include "var_metadata.h"
#include "includes/logger/entity_logger.h"

class VarTableNode
{
public:
    VarTableNode(VarTableNode* parent, const EntityLogger& entityLogger);
    VarTableNode() = delete;
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
    std::optional<VarMetaData> FindVar(const std::string& sVarName) const;

    // The func return the stackOffset.
    int GetStackOffset() const;

    // The func returns the block at the given index.
    // @return: nullpt -> The block doesnt exist. Otherwise -> The block at the given index.
    VarTableNode* GetBlock(int nBlockIndex) const;
    VarTableNode* GetParent() const noexcept;

    // Getters
    const std::unordered_map<std::string, VarMetaData>& GetVars() const noexcept;
    const std::vector<std::unique_ptr<VarTableNode>>& GetChildren() const noexcept;

    // Operators
    VarTableNode& operator[](size_t index);

    const VarTableNode& operator[](size_t index) const;

private:
    std::unordered_map<std::string, VarMetaData> m_vars;
    VarTableNode* m_parent;
    std::vector<std::unique_ptr<VarTableNode>> m_children;
    int m_nCurrentStackOffset;
    const EntityLogger& m_entityLogger;
};
