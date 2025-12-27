#include "includes/semantic_analysis/semantic_checker_facade.h"

bool SemanticCheckerFacade::CheckVarDeclaration(VarTableNode currVarTableNode, VarMetaData declaredVar)
{
    if (currVarTableNode.FindVar(declaredVar.sName)) return false;
    return true;
}

bool SemanticCheckerFacade::CheckFuncDeclaration(FuncTable funcTable, FuncMetaData declaredFunc)
{
    if (funcTable.FindFunctionByName(declaredFunc.GetName())) return false;
    return true;
}
