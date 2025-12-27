#pragma once

#include <vector>

#include "var_table_node.h"
#include "func_table.h"
#include "semantic_consts.h"
#include "includes/parser/command.h"

// @note arrays are not supported at this point in time
// @check if EntityLogger should be included
// @check what to do with errors
// @check what about control flows

// @note No one of the funcs checks that a var or func exist it's the user's responsebility (the funcs are: CheckExpression, CheckFuncArguments, CheckFunctionReturnType, CheckAssignment)
class SemanticCheckerFacade
{
public:
    SemanticCheckerFacade() = delete;
    
    // @brief The func checks if the declaredVar declaration is legal.
    // @param currVarTable The VarTableNode the var is created in.
    // @param declaredVar The metadata of the var being declared.
    // @return bool: True-> Declaration is legal. False-> Otherwise.
    static bool CheckVarDeclaration(const VarTableNode& currVarTableNode, const VarMetaData& declaredVar);

    // @brief The func checks if the declaredFunc declaration is legal.
    // @param funcTable The FuncTable.
    // @param declaredFunc The metadata of the func being declared.
    // @return bool: True-> Declaration is legal. False-> Otherwise.
    static bool CheckFuncDeclaration(const FuncTable& funcTable, const FuncMetaData& declaredFunc);

    // @brief The func checks if the usedVar exists.
    // @param currVarTable The VarTableNode in up to this point.
    // @param usedVar The name of the var used.
    // @return bool: True-> Var exists. False-> Otherwise.
    static bool CheckVarExistence(const VarTableNode& currVarTableNode, const std::string& usedVar);

    // @brief The func checks if the usedFunc exists.
    // @param funcTable The FuncTable.
    // @param usedFunc The name of the func called.
    // @return bool: True-> Func exists. False-> Otherwise.
    static bool CheckFuncExistence(const FuncTable& funcTable, const std::string& usedFunc);

    // @brief The func is used to check that the expression is legal.
    // @param currExpressionPart The current checked Command in the expression.
    // @param funcTable The funcTable incase the Command is a func call.
    // @param currVarTableNode The varTable incase the Command is a var use.
    // @param lastType The last known type of the expression.
    // @return ExpressionType: Illegal-> UNKNOWN, Legal-> Otherwise.
    // @note if returned UNKNOWN dont continue checking this.
    static ExpressionType CheckExpression(const Command& currExpressionPart,
        const FuncTable& funcTable,
        const VarTableNode& currVarTableNode,
        ExpressionType lastType = ExpressionType::UNKNOWN);
    
    // @brief The func checks that the functions arguments are correct.
    // @param paramTypes The types that were given to the func.
    // @param sFuncName The func name.
    // @param funcTable The func table.
    // @return bool: True-> The given arguments are legal, False-> Otherwise.
    static bool CheckFuncArguments(const std::vector<ExpressionType>& paramTypes, const std::string& sFuncName, const FuncTable& funcTable);

    // @brief The func checks that the return is the correct type.
    // @param returnExpressionType The returnType we got.
    // @param sFuncName The func name.
    // @param funcTable The func table.
    // @return bool: True-> The rtype is correct, False-> Otherwise. 
    static bool CheckFunctionReturnType(ExpressionType returnExpressionType, const std::string& sFuncName, const FuncTable& funcTable);

    // @brief The func is used to check if the assignment is legal.
    // @param sAssignedVarName The var that is being assigned a new value.
    // @param assigmentExpressionType The Type that is being assigned
    // @param currVarTableNode The current var table we are in.
    // @return bool: True-> The assignment is legal, False-> Otherwise.
    static bool CheckAssignment(const std::string& sAssignedVarName, const ExpressionType& assigmentExpressionType, const VarTableNode& currVarTableNode);

private:
    // @brief The func checks the current type and the lastType.
    // @return ExpressionType: UNKNOWN-> There was a problem, Otherwise-> Legal.
    static ExpressionType CheckExpressionKnownType(ExpressionType lastType, Type objectType);

    // @brief The func changes a Token into and ExpressionType.
    // @return ExpressionType: the corresponding ExpressionType for the Token.
    static ExpressionType TokenToExpressionType(const Token& curr);

    // @brief The func Compares the expected and gotten type.
    // @return bool: True-> The types match, False-> Otherwise.
    static bool CompareTypes(Type expectedType, ExpressionType gottenType);
};
