#pragma once
#include <string>

#include "includes/common/common_consts.h"

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
        int offset
    ): 
        sName(name),
        type(type),
        accessRights(access),
        scope(scope),
        offsetInsideFrame(offset)
    {}

    std::string sName;
    Type type;
    AccessRights accessRights;
    Scope scope;
    int offsetInsideFrame;
};
