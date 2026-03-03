#pragma once

#include "var_metadata.h"
#include "var_table_node.h"
#include "includes/shared/common/target_architecture.h"
#include "includes/frontend/common/project_symbol_table.h"

constexpr int NORMAL_32BIT_TYPE_SIZE = 4;
constexpr int NORMAL_64BIT_TYPE_SIZE = 8;

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
        Scope varScope
    ) const;

    VarMetaData CreateVarMetadata(const ExportedVar& exportedVar) const;

private:
    const EntityLogger& m_entityLogger;
};