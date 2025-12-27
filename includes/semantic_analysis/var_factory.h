#pragma once

#include "var_metadata.h"
#include "var_table_node.h"

class VarFactory
{
public:
    VarFactory() = default;

    VarTableNode CreateNewVarTable() const;

    VarMetaData CreateVarMetadata(
        std::string sVarName,
        Type varType,
        AccessRights varAccessRights,
        Scope varScope,
        int varOffsetInsideFrame
    ) const;
};