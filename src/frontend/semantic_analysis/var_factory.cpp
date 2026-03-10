#include "includes/frontend/semantic_analysis/var_factory.h"

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
    std::optional<size_t> arraySize
) const
{
    int elementSize = 0;
    switch (varType)
    {
    case Type::BOOL_TYPE:
    case Type::CHAR_TYPE:
        elementSize = 1;
        break;
    case Type::DOUBLE_TYPE:
        elementSize = TargetArchitecture::Is32Bit() ? NORMAL_32BIT_TYPE_SIZE : NORMAL_64BIT_TYPE_SIZE;
        break;
    case Type::INT_TYPE:
    case Type::UNSIGNED_INT_TYPE:
    case Type::STRING_TYPE:
        elementSize = TargetArchitecture::Is32Bit() ? NORMAL_32BIT_TYPE_SIZE : NORMAL_64BIT_TYPE_SIZE;
        break;
    default:
        break;
    }

    int varOffsetInsideFrame = arraySize.has_value()
        ? static_cast<int>(elementSize * arraySize.value())
        : elementSize;

    return VarMetaData(
        sVarName,
        varType,
        varAccessRights,
        varScope,
        varOffsetInsideFrame,
        false,
        arraySize
    );
}

VarMetaData VarFactory::CreateVarMetadata(const ExportedVar& exportedVar) const
{
    int varOffsetInsideFrame = 0;
    switch (exportedVar.type)
    {
    case Type::BOOL_TYPE:
    case Type::CHAR_TYPE:
        varOffsetInsideFrame = 1;
        break;
    
    case Type::DOUBLE_TYPE:
        if (TargetArchitecture::Is32Bit())
        {
            varOffsetInsideFrame = NORMAL_32BIT_TYPE_SIZE;
        }
        else
        {
            varOffsetInsideFrame = NORMAL_64BIT_TYPE_SIZE;
        }
        break;

    case Type::INT_TYPE:
    case Type::UNSIGNED_INT_TYPE:
    case Type::STRING_TYPE:  // String vars hold a pointer to the character data, not the content itself
        if (TargetArchitecture::Is32Bit())
        {
            varOffsetInsideFrame = NORMAL_32BIT_TYPE_SIZE;
        }
        else
        {
            varOffsetInsideFrame = NORMAL_64BIT_TYPE_SIZE;
        }
        break;

    default:
        break;
    }

    return VarMetaData(
        exportedVar.name,
        exportedVar.type,
        exportedVar.accessRights,
        Scope::GLOBAL,
        varOffsetInsideFrame,
        true
    );
}