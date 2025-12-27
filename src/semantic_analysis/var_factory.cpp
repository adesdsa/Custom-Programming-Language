#include "includes/semantic_analysis/var_factory.h"

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