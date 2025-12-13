#pragma once
#include <string>
#include "semantic_consts.h"


// Usage example:
//  scope->currentStackOffset = 0; // start of function
//  
//  // declare first local
//  VarMetaData x;  // x is int variable
//  scope->currentStackOffset -= sizeof(int);  // -4
//  x.offsetInsideFrame= scope->currentStackOffset;
typedef struct VarMetaData {
    std::string sName;
    Type type;
    AccessRights accessRights;
    Scope scope;
    int offsetInsideFrame;
} VarMetaData;

struct VarTableNode
{
    std::unordered_map<std::string, VarMetaData> vars;
    VarTableNode* parent = nullptr;
    std::vector<VarTableNode*> children;
    int currentStackOffset = 0; 
};
