#pragma once

#include <vector>

#include "var_table_node.h"
#include "func_table.h"
#include "semantic_consts.h"
#include "includes/parser/command.h"

// @todo check how to know if funcs exist if they are used before their declaration.
// @note splitting the parser to make it so that any value is has_param and operators are between them might be good.
// @todo check if arrays are supported
// @todo recheck () in an expression

// @note No one of the func's checks that a var or func exists it's the user's responsebility (e.g. checkExpression doesnt check)
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

    static ExpressionType CheckExpression(const Command& currExpressionPart,
        const FuncTable& funcTable,
        const VarTableNode& currVarTableNode,
        ExpressionType lastType = ExpressionType::UNKNOWN);
    
    static bool CheckFuncArguements(std::vector<ExpressionType> paramTypes, const std::string& sFuncName, const FuncTable& funcTable);

private:
    static ExpressionType CheckExpressionKnownType(ExpressionType lastType, Type objectType);
    static ExpressionType TokenToExpressionType(const Token& curr);
};
