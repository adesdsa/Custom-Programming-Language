#include "includes/semantic_analysis/var_factory.h"

VarFactory::VarFactory(const EntityLogger& entityLogger):
    m_entityLogger(entityLogger)
{
}

VarTableNode VarFactory::CreateNewVarTable() const
{
    return VarTableNode(nullptr, m_entityLogger);
}

VarMetaData VarFactory::CreateVarMetadata(
    std::string sVarName,
    Type varType,
    AccessRights varAccessRights,
    Scope varScope,
    int varOffsetInsideFrame
) const
{
    return VarMetaData(
        sVarName,
        varType,
        varAccessRights,
        varScope,
        varOffsetInsideFrame
    );
}