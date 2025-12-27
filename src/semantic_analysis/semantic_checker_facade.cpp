#include "includes/semantic_analysis/semantic_checker_facade.h"

static bool SemanticCheckerFacade::CheckVarDeclaration(VarTableNode currVarTableNode, VarMetaData declaredVar)
{
    if (currVarTableNode.FindVar(declaredVar.sName)) return false;
    return true;
}

static bool SemanticCheckerFacade::CheckFuncDeclaration(FuncTable funcTable, FuncMetaData declaredFunc)
{
    if (funcTable.FindFunctionByName(declaredFunc.GetName())) return false;
    return true;
}

static bool SemanticCheckerFacade::CheckVarExistence(const VarTableNode& currVarTableNode, const std::string& usedVar)
{
    if (currVarTableNode.FindVar(usedVar)) return true;
    return false;
}

static bool SemanticCheckerFacade::CheckFuncExistence(const FuncTable& funcTable, const std::string& usedFunc)
{
    if (funcTable.FindFunctionByName(usedFunc)) return true;
    return false;
}
