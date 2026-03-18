#pragma once

#include <vector>
#include <stack>

#include "var_table_node.h"
#include "func_table.h"
#include "includes/shared/common/common_consts.h"
#include "includes/frontend/parser/command.h"

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

    SemanticCheckerFacade();

    /// @brief The func checks if the varName exists.
    /// @param currVarTable The VarTableNode in up to this point.
    /// @param varName The name of the var used.
    /// @return bool: True-> Var exists. False-> Otherwise.
    bool CheckVarExistence(const VarTableNode& currVarTableNode, const std::string& varName);

    /// @brief The func checks if the funcName declared.
    /// @param funcTable The FuncTable.
    /// @param funcName The name of the func called.
    /// @return bool: True-> Func declared. False-> Otherwise.
    bool CheckFuncDeclaration(const FuncTable& funcTable, const std::string& funcName);

    /// @brief The func checks if the funcName defined.
    /// @param funcTable The FuncTable.
    /// @param funcName The name of the func called.
    /// @return bool: True-> Func defined. False-> Otherwise.
    bool CheckFuncDefinition(const FuncTable& funcTable, const std::string& funcName);

    /// @brief The func is used to check that the expression is legal.
    /// @param currExpressionPart The current checked Command in the expression.
    /// @param funcTable The funcTable incase the Command is a func call.
    /// @param currVarTableNode The varTable incase the Command is a var use.
    /// @return ExpressionType: Illegal-> UNKNOWN, Legal-> Otherwise.
    /// @note The func does check that vars and funcs exist and have correct type but it doesnt check that 
    /// the functions gets the right parameters it is your responsebility.
    ExpressionType CheckExpression(const Command& expression,
        const FuncTable& funcTable,
        const VarTableNode& currVarTableNode);
    
    /// @brief The func checks that the functions arguments are correct.
    /// It is used by getting the ExpressionTypes of all the arguements and then returning them in a vector.
    /// @param paramTypes The types of the func args.
    /// @param sFuncName The func name.
    /// @param funcTable The func table.
    /// @return bool: True-> The given arguments are legal, False-> Otherwise.
    bool CheckFuncArguments(const std::vector<ExpressionType>& paramTypes, const std::string& sFuncName, const FuncTable& funcTable);

    /// @brief The func checks that the return is the correct type.
    /// @param returnExpressionType The returnType we got from the returned expression.
    /// @param sFuncName The func name.
    /// @param funcTable The func table.
    /// @return bool: True-> The rtype is correct, False-> Otherwise.
    bool CheckFunctionReturnType(const ExpressionType& returnExpressionType, const std::string& sFuncName, const FuncTable& funcTable);

    /// @brief The func is used to check that the return is inside a func.
    /// @note The func must be checked before calling CheckFunctionReturnType.
    /// @return bool: True-> The return is inside a func, False-> Otherwise.
    bool CheckReturnInsideFunc();

    /// @brief The func is used to check if the assignment is legal.
    /// @param sAssignedVarName The var that is being assigned a new value.
    /// @param assigmentExpressionType The Type that is being assigned
    /// @param currVarTableNode The current var table we are in.
    /// @return bool: True-> The assignment is legal, False-> Otherwise.
    bool CheckAssignment(const std::string& sAssignedVarName, const ExpressionType& assigmentExpressionType, const VarTableNode& currVarTableNode, bool isInitialization = false);

    /// @brief Checks if the expression is a compile-time constant (literals, const vars, constant expressions only).
    /// Used to enforce that const variables are initialized only with compile-time known values, not e.g. read_int().
    /// @param expression The initializer expression.
    /// @param funcTable The function table (to reject any function calls).
    /// @param currVarTableNode The var table (to allow const identifiers as constants).
    /// @return bool: True if the expression is compile-time constant, false otherwise.
    bool IsCompileTimeConstant(const Command& expression, const FuncTable& funcTable, const VarTableNode& currVarTableNode);

    /// @brief The func checks if a control statement is legal right now.
    /// @return bool: True-> Control statement is legal, False-> Otherwise.
    bool CheckControlStatement();

    /// @brief The func is used when you enter a block to check that all control paths return a value.
    /// @param entryKind What block did we enter.
    /// @param bIsReturnVoid Incase it's a func that returns void do true.
    /// @return bool: False-> Started an else or elif block not after if or elif, True-> Otherwise.
    /// @note If this isnt the start of elif or else the return value doesnt matter.
    bool EnteredBlock(const BlockKind& entryKind, bool bIsReturnVoid);

    /// @brief The func is used to handle closing a block and do the according checks.
    /// @return bool: False-> Not all code paths return a value, True-> Otherwise.
    /// @note If this isnt the end of the func the return value doesnt matter.
    bool ExitedBlock();

    /// @brief The func checks if the block is finished.
    /// @note Please notice that you have to use this func before you try adding more operations to the current block.
    /// This is incase there is dead code after finish.
    /// Incase it returns true and there is more code and not a block finish go to the end of the block and continue checking from there.
    /// You may call it an error or not just jump to the end of the block either way.
    /// @return bool: True-> Block returned therefore it is finished, False-> Otherwise.
    bool IsBlockFinished();

    /// @brief Check that all functions that have been declared are also defined
    /// @param funcTable Table of all the the declared functions 
    /// @return vector<FuncMetaData*>: Vector of pointers to all functions that are not defined
    /// @note Incase that all functions are defined vector will be empty
    /// @note This function should be call after finish visitor logic
    std::vector<FuncMetaData*> AreAllFunctionsDefined(const FuncTable& funcTable);

    /// @brief The func turns a Type into ExpressionType
    ExpressionType TypeToExpressionType(const Type& type);

    /// @brief Returns true if the given ARRAY_SLICE is applied to a string (simple or chained).
    ///        Slicing is only legal on strings.
    bool IsSliceBaseString(const Command& sliceCommand,
        const FuncTable& funcTable, const VarTableNode& currVarTableNode);

    /// @brief Suppress DefaultExpressionChecks so that condition evaluation
    /// @note(if/elif/while headers) does not overwrite the operation history.
    void SetSuppressDefaultExprCheck(bool suppress);

private:
    enum class OperationKind // This class repressends the last operation that happened in the block.
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

    std::stack<OperationKind> m_operationHistory; // The last operation that happened in a block level(block inside block).
    std::stack<BlockKind> m_entryHistory;        // The last block that was entered.
    size_t m_uLoopBlockCnt;                     // A counter that represents the amount of blocks entered when in a loop.
    bool m_bFinishedBlock;                     // A flag that represents if we finished the current block (if there was a 100% return).
    bool m_bIsReturnVoid;                     // A flag that represents if the function returns void.
    bool m_bLoopFlag;                        // A flag that represents if we are inside a loop rn (if true no need to do control flow checks).
    bool m_bSuppressDefaultExprCheck;       // When true, DefaultExpressionChecks is a no-op (used during condition evaluation).

    /// @brief Returns true if the first token of the expression is string-like or char-like (string/char literal, string/char var, or string-returning func).
    bool IsFirstTokenStringOrCharLike(const Command& expression, const FuncTable& funcTable, const VarTableNode& currVarTableNode);
    /// @brief Helper for IsFirstTokenStringOrCharLike that works on a token range (handles leading parentheses).
    bool IsFirstTokenStringOrCharLikeFrom(const Command& expression, size_t start, size_t end, size_t& paramIndex,
        const FuncTable& funcTable, const VarTableNode& currVarTableNode);

    /// @brief The func changes a Token into and ExpressionType.
    /// @return ExpressionType: the corresponding ExpressionType for the Token.
    ExpressionType TokenToExpressionType(const Token& curr);

    /// @brief The func Compares the expected and gotten type.
    /// @return bool: True-> The types match, False-> Otherwise.
    bool CompareTypes(const Type& expectedType, const ExpressionType& gottenType);

    /// @brief The func does the default checks before an expression assignment as a current operation.
    /// There are three possible reasons why won't need to store the operations:
    /// 1. We are inside a loop. 2. We arent inside a func. 3. The func is already finished (after return).
    inline void DefaultExpressionChecks();

    /// @brief The func is used to handle a block enter in either else or elif.
    /// @return bool: False-> elif or else not after a condition block(if or elif). True-> Otherwise.
    bool ElseElifBlockEnter();

    /// @brief The func is used to handle a block exit in either else or elif.
    /// @param kind Either else or elif.
    void ElseElifBlockExit(OperationKind kind);

    /// @brief The func is used to switch the top of the m_operationHistory stack.
    /// It is used to ensure that we only change the top if there is one, and it automatically pops and then pushed to change.
    /// @param operationKind The operation we add.
    inline void SwitchTopOfOperationHistory(OperationKind operationKind);

    /// @brief The func is used to parse all the parantheses into a simple epxressionType vector.
    /// e.g. true and (1 < 2) will be passed as next arguemnt -> 
    /// BOOLEAN BOOLEAN PARANTHESES_OPEN MATHEMATICAL COMPARRISON_OPERATOR MATHEMATICAL PARANTHESES_CLOSE
    /// this will then be returned as -> BOOLEAN BOOLEAN BOOLEAN after simplifying.
    /// @param parsedExpression The expression after it was parsed from tokens to ExpressionType.
    /// @return std::vector<ExpressionType> -> [ExpressionType::UNKNOWN] if found illegal combination, 
    /// Simplified vector without parantheses if legal.
    std::vector<ExpressionType> SimplifyExpressionParantheses(std::vector<ExpressionType> parsedExpression);

    /// @brief The func simplifies an expression without parantheses into a single type.
    /// e.g. if we have the expression 1 < 2 which is MATHEMATICAL COMPARISSON_OPERATOR MATHEMATICAL
    /// it will turn to BOOLEAN.
    /// @param expressionSet A vector of ExpressionTypes that represents an expression without any parantheses.
    /// @return The ExpressionType that the expression represents.
    ExpressionType SimplifyExpressionSet(std::vector<ExpressionType> expressionSet);
};
