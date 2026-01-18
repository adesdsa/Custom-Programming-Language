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
    Scope varScope
) const
{
    int varOffsetInsideFrame = 0;
    switch (varType)
    {
    case Type::BOOL_TYPE:
        varOffsetInsideFrame = sizeof(bool);
        break;
    
    case Type::DOUBLE_TYPE:
        varOffsetInsideFrame = sizeof(double);
        break;

    case Type::INT_TYPE:
        varOffsetInsideFrame = sizeof(int);
        break;

    case Type::UNSIGNED_INT_TYPE:
        varOffsetInsideFrame = sizeof(int);
        break;

    default:
        break;
    }

    return VarMetaData(
        sVarName,
        varType,
        varAccessRights,
        varScope,
        varOffsetInsideFrame
    );
}