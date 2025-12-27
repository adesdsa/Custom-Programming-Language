#pragma once

#include "var_table_node.h"
#include "func_table.h"

class SemanticCheckerFacade
{
public:
    SemanticCheckerFacade() = delete;
    
    // @brief: The func checks if the declaredVar declaration is legal.
    // @param currVarTable: The VarTableNode the var is created in.
    // @param declaredVar: The metadata of the var being declared.
    // @return bool: True-> Declaration is legal. False-> Otherwise.
    bool CheckVarDeclaration(VarTableNode currVarTableNode, VarMetaData declaredVar);

    // @brief: The func checks if the declaredFunc declaration is legal.
    // @param funcTable: The FuncTable.
    // @param declaredFunc: The metadata of the func being declared.
    // @return bool: True-> Declaration is legal. False-> Otherwise.
    bool CheckFuncDeclaration(FuncTable funcTable, FuncMetaData declaredFunc);
};
