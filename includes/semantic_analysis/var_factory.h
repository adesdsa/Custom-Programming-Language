#pragma once

#include "var_metadata.h"

class VarFactory
{
public:
    VarFactory() = default;

    VarMetaData CreateVarMetadata(
        std::string sVarName,
        Type varType,
        AccessRights varAccessRights,
        Scope varScope,
        int varOffsetInsideFrame
    ) const;
};