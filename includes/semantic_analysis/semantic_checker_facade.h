#pragma once

#include <vector>
#include <stack>

#include "var_table_node.h"
#include "func_table.h"
#include "semantic_consts.h"
#include "includes/parser/command.h"

/// @note No one of the funcs checks that a var or func exist it's the developer's responsebility (the funcs are: CheckFuncArguments, CheckFunctionReturnType, CheckAssignment).
/// @note Before using please read all checks and learn where they need to be used.
class SemanticCheckerFacade
{
public:
    enum class BlockKind
    {
        Function,
        Loop,
        If,
        Elif,
        Else
    };

    SemanticCheckerFacade() = delete;

    /// @brief The func checks if the varName exists.
    /// @param currVarTable The VarTableNode in up to this point.
    /// @param varName The name of the var used.
    /// @return bool: True-> Var exists. False-> Otherwise.
    static bool CheckVarExistence(const VarTableNode& currVarTableNode, const std::string& varName);

    /// @brief The func checks if the funcName declared.
    /// @param funcTable The FuncTable.
    /// @param funcName The name of the func called.
    /// @return bool: True-> Func declared. False-> Otherwise.
    static bool CheckFuncDeclaration(const FuncTable& funcTable, const std::string& funcName);

    /// @brief The func checks if the funcName defined.
    /// @param funcTable The FuncTable.
    /// @param funcName The name of the func called.
    /// @return bool: True-> Func defined. False-> Otherwise.
    static bool CheckFuncDefinition(const FuncTable& funcTable, const std::string& funcName);

    /// @brief The func is used to check that the expression is legal.
    /// @param currExpressionPart The current checked Command in the expression.
    /// @param funcTable The funcTable incase the Command is a func call.
    /// @param currVarTableNode The varTable incase the Command is a var use.
    /// @return ExpressionType: Illegal-> UNKNOWN, Legal-> Otherwise.
    /// @note The func does check that vars and funcs exist and have correct type but it doesnt check that 
    /// the functions gets the right parameters it is your responsebility.
    static ExpressionType CheckExpression(const Command& expression,
        const FuncTable& funcTable,
        const VarTableNode& currVarTableNode);
    
    /// @brief The func checks that the functions arguments are correct.
    /// It is used by getting the ExpressionTypes of all the arguements and then returning them in a vector.
    /// @param paramTypes The types of the func args.
    /// @param sFuncName The func name.
    /// @param funcTable The func table.
    /// @return bool: True-> The given arguments are legal, False-> Otherwise.
    static bool CheckFuncArguments(const std::vector<ExpressionType>& paramTypes, const std::string& sFuncName, const FuncTable& funcTable);

    /// @brief The func checks that the return is the correct type.
    /// @param returnExpressionType The returnType we got from the returned expression.
    /// @param sFuncName The func name.
    /// @param funcTable The func table.
    /// @return bool: True-> The rtype is correct, False-> Otherwise.
    static bool CheckFunctionReturnType(const ExpressionType& returnExpressionType, const std::string& sFuncName, const FuncTable& funcTable);

    /// @brief The func is used to check that the return is inside a func.
    /// @note The func must be checked before calling CheckFunctionReturnType.
    /// @return bool: True-> The return is inside a func, False-> Otherwise.
    static inline bool CheckReturnInsideFunc();

    /// @brief The func is used to check if the assignment is legal.
    /// @param sAssignedVarName The var that is being assigned a new value.
    /// @param assigmentExpressionType The Type that is being assigned
    /// @param currVarTableNode The current var table we are in.
    /// @return bool: True-> The assignment is legal, False-> Otherwise.
    static bool CheckAssignment(const std::string& sAssignedVarName, const ExpressionType& assigmentExpressionType, const VarTableNode& currVarTableNode);

    /// @brief The func checks if a control statement is legal right now.
    /// @return bool: True-> Control statement is legal, False-> Otherwise.
    static bool CheckControlStatement();

    /// @brief The func is used when you enter a block to check that all control paths return a value.
    /// @param entryKind What block did we enter.
    /// @param bIsReturnVoid Incase it's a func that returns void do true.
    /// @return bool: False-> Started an else or elif block not after if or elif, True-> Otherwise.
    /// @note If this isnt the start of elif or else the return value doesnt matter.
    static bool EnteredBlock(const BlockKind& entryKind, bool bIsReturnVoid);

    /// @brief The func is used to handle closing a block and do the according checks.
    /// @return bool: False-> Not all code paths return a value, True-> Otherwise.
    /// @note If this isnt the end of the func the return value doesnt matter.
    static bool ExitedBlock();

    /// @brief The func checks if the block is finished.
    /// @note Please notice that you have to use this func before you try adding more operations to the current block.
    /// This is incase there is dead code after finish.
    /// Incase it returns true and there is more code and not a block finish go to the end of the block and continue checking from there.
    /// You may call it an error or not just jump to the end of the block either way.
    /// @return bool: True-> Block returned therefore it is finished, False-> Otherwise.
    static bool IsBlockFinished();

private:
enum class OperationKind
    {
        None,
        Return,
        If,
        Elif,
        Else,
        IfReturn,
        ElifReturn,
        ElseReturn,
        Expression
    };

    /// @todo add docs for all params
    static bool m_bLoopFlag;
    static bool m_bFinishedBlock;
    static size_t m_uLoopBlockCnt;
    static std::stack<OperationKind> m_operationHistory;
    static std::stack<BlockKind> m_entryHistory;
    static bool m_bIsReturnVoid;

    /// @brief The func turns a Type into ExpressionType
    static ExpressionType TypeToExpressionType(const Type& type);

    /// @brief The func changes a Token into and ExpressionType.
    /// @return ExpressionType: the corresponding ExpressionType for the Token.
    static ExpressionType TokenToExpressionType(const Token& curr);

    /// @brief The func Compares the expected and gotten type.
    /// @return bool: True-> The types match, False-> Otherwise.
    static bool CompareTypes(const Type& expectedType, const ExpressionType& gottenType);

    /// @brief The func does the default checks before an expression assignment as a current operation.
    /// There are three possible reasons why won't need to store the operations:
    /// 1. We are inside a loop. 2. We arent inside a func. 3. The func is already finished (after return).
    static inline void DefaultExpressionChecks();

    /// @brief The func is used to handle a block enter in either else or elif.
    /// @return bool: False-> elif or else not after a condition block(if or elif). True-> Otherwise.
    static bool ElseElifBlockEnter();

    /// @brief The func is used to handle a block exit in either else or elif.
    /// @param kind Either else or elif.
    static void ElseElifBlockExit(OperationKind kind);

    /// @brief The func is used to switch the top of the m_operationHistory stack.
    /// @param operationKind The operation we add.
    static void SwitchTop(OperationKind operationKind);
};
