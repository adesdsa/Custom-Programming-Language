#pragma once
#include <string>
#include <optional>

#include "includes/shared/common/common_consts.h"

// Usage example:
//  scope->currentStackOffset = 0; // start of function
//  
//  // declare first local
//  VarMetaData x;  // x is int variable
//  scope->currentStackOffset -= sizeof(int);  // -4
//  x.offsetInsideFrame= scope->currentStackOffset;
struct VarMetaData 
{
    VarMetaData
    (
        const std::string& name,
        Type type,
        AccessRights access,
        Scope scope,
        int offset,
        bool isExtern = false,
        std::optional<size_t> arraySize = std::nullopt
    ): 
        sName(name),
        type(type),
        accessRights(access),
        scope(scope),
        offsetInsideFrame(offset),
        isExtern(isExtern),
        arraySize(arraySize)
    {}

    bool operator==(const VarMetaData& other) const
    {
        return sName == other.sName &&
               type == other.type &&
               accessRights == other.accessRights &&
               scope == other.scope &&
               offsetInsideFrame == other.offsetInsideFrame &&
               isExtern == other.isExtern &&
               arraySize == other.arraySize;
    }

    std::string sName;
    Type type;
    AccessRights accessRights;
    Scope scope;
    int offsetInsideFrame;
    bool isExtern;
    std::optional<size_t> arraySize;  // When set, variable is a static array of this many elements
};
