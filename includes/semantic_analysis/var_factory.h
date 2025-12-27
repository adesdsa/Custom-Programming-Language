#pragma once

#include "var_metadata.h"
#include "var_table_node.h"

class VarFactory
{
public:
    VarFactory(const EntityLogger& entityLogger);

    // @brief: Func create a new brand new var table (a new root).
    // @return VarTableNode: The new var table node.
    VarTableNode CreateNewVarTable() const;

    // @brief: The func create a new var metadata.
    // @return VarMetaData: A new VarMetadata object.
    // @note: please add to the varOffsetInsideFrame the type_size+buffer.
    VarMetaData CreateVarMetadata(
        std::string sVarName,
        Type varType,
        AccessRights varAccessRights,
        Scope varScope,
        int varOffsetInsideFrame
    ) const;

private:
    const EntityLogger& m_entityLogger;
};