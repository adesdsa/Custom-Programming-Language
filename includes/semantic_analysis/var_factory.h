#pragma once

#include "var_metadata.h"

class VarFactory
{
public:
    VarFactory(const EntityLogger& entityLogger);

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