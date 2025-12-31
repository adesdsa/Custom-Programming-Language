#include "includes/semantic_analysis/semantic_checker_facade.h"

bool SemanticCheckerFacade::CheckVarExistence(const VarTableNode& currVarTableNode, const std::string& varName)
{
    return currVarTableNode.FindVar(varName) != std::nullopt;
}

bool SemanticCheckerFacade::CheckFuncExistence(const FuncTable& funcTable, const std::string& funcNmae)
{
    return funcTable.FindFunctionByName(funcNmae)->IsFuncDeclared();
}

static bool CheckFuncDeclaration(const FuncTable& funcTable, const std::string& funcNmae)
{
    return funcTable.FindFunctionByName(funcNmae)->IsFuncDefined();
}

ExpressionType SemanticCheckerFacade::CheckExpression(
    const Command& currExpressionPart,
    const FuncTable& funcTable,
    const VarTableNode& currVarTableNode,
    ExpressionType lastType)
{
    /// @note: The semantic checker doesnt distuinguish between INT and DOUBLE only between MATH and BOOL, That's why it only uses DOUBLE_TYPE
    ExpressionType currType;

    if (currExpressionPart.GetType() == CommandTypes::EXPRESSION)
    {
        for (size_t i = 0; i < currExpressionPart.GetTokens().size(); i++)
        {
            /// No need to check parantheses.
            if (currExpressionPart[i].GetValue() != "(" && currExpressionPart[i].GetValue() != ")")
            {
                currType = TokenToExpressionType(currExpressionPart[i]);
                if (lastType == ExpressionType::UNKNOWN) lastType = currType;
                else if (lastType != currType) return ExpressionType::UNKNOWN;
            }
        }
        return currType;
    }
    else if (currExpressionPart.GetType() == CommandTypes::IDENTIFIER_VALUE)
    {
        auto varData = currVarTableNode.FindVar(currExpressionPart[0].GetValue());
        return CheckExpressionKnownType(lastType, varData->type);
    }
    else if (currExpressionPart.GetType() == CommandTypes::FINISHED_FUNC_CALL)
    {
        auto funcData = funcTable.FindFunctionByName(currExpressionPart[0].GetValue());
        return CheckExpressionKnownType(lastType, funcData->GetRType());
    }
}

bool SemanticCheckerFacade::CheckFuncArguments(const std::vector<ExpressionType>& paramTypes, const std::string& sFuncName, const FuncTable& funcTable)
{
    const std::vector<VarMetaData>& funcParams = funcTable.FindFunctionByName(sFuncName)->GetFuncParams();
    if (funcParams.size() != paramTypes.size()) return false;
    if (funcParams[0].type == Type::VOID_TYPE) return true; /// Void as a param is a hardcoded check inside the parser.
    for (size_t i = 0; i < paramTypes.size(); i++)
    {
        if (!CompareTypes(funcParams[i].type, paramTypes[i])) return false;
    }
    return true;
}

bool SemanticCheckerFacade::CheckFunctionReturnType(ExpressionType returnExpressionType, const std::string& sFuncName, const FuncTable& funcTable)
{
    return CompareTypes(funcTable.FindFunctionByName(sFuncName)->GetRType(), returnExpressionType);
}

bool SemanticCheckerFacade::CheckAssignment(const std::string& sAssignedVarName, const ExpressionType& assigmentExpressionType, const VarTableNode& currVarTableNode)
{
    VarMetaData varData = *currVarTableNode.FindVar(sAssignedVarName);
    if (varData.accessRights == AccessRights::CONST_ACCESS || varData.accessRights == AccessRights::READ_ONLY_ACCESS) return false;
    return CompareTypes(varData.type, assigmentExpressionType);
}

ExpressionType SemanticCheckerFacade::CheckExpressionKnownType(ExpressionType lastType, Type objectType)
{
    if (objectType == Type::VOID_TYPE) return ExpressionType::VOID_TYPE;
    else if (objectType == Type::BOOL_TYPE)
    {
        if (lastType == ExpressionType::UNKNOWN) return ExpressionType::BOOLEAN;
        else if (lastType == ExpressionType::BOOLEAN) return ExpressionType::BOOLEAN;
        else return ExpressionType::UNKNOWN;
    }
    else
    {
        if (lastType == ExpressionType::UNKNOWN) return ExpressionType::MATHEMATICAL;
        else if (lastType == ExpressionType::MATHEMATICAL) return ExpressionType::MATHEMATICAL;
        else return ExpressionType::UNKNOWN;
    }
}

ExpressionType SemanticCheckerFacade::TokenToExpressionType(const Token& token)
{
    /// First we check if the token is an operator
    if (token.GetValue() != "=" && token.GetType() == TokenTypes::OPERATOR || token.GetType() == TokenTypes::KEYWORD)
    {
        if (token.GetValue() == "==" ||
            token.GetValue() == "and" ||
            token.GetValue() == "or" ||
            token.GetValue() == "not" ||
            token.GetValue() == "xor")
        {
            return ExpressionType::BOOLEAN;
        }
        else return ExpressionType::MATHEMATICAL;
    }
    /// If it's a value
    else if (token.GetValue() == "true" || token.GetValue() == "false")
    {
        return ExpressionType::BOOLEAN;
    }
    else return ExpressionType::MATHEMATICAL;
}

bool SemanticCheckerFacade::CompareTypes(Type expectedType, ExpressionType gottenType)
{
    if (gottenType == ExpressionType::UNKNOWN) return false;
    if (expectedType == Type::BOOL_TYPE && gottenType == ExpressionType::BOOLEAN) return true;
    if (expectedType == Type::VOID_TYPE && gottenType == ExpressionType::VOID_TYPE) return true;
    if (gottenType == ExpressionType::MATHEMATICAL && (
        expectedType == Type::DOUBLE_TYPE ||
        expectedType == Type::INT_TYPE ||
        expectedType == Type::UNSIGNED_DOUBLE_TYPE ||
        expectedType == Type::UNSIGNED_INT_TYPE)) return true;

    return false;
}
