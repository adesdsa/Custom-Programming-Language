#pragma once

#include <stack>
#include <vector>
#include <initializer_list>

#include "includes/shared/common/abstract_ast_visitor.h"
#include "includes/shared/common/ast_helpers/decorated_ast.h"
#include "includes/shared/errors/error_manager.h"
#include "includes/backend/codegen/ir/ir_builder.h"
#include "includes/backend/codegen/ir/ir_templates/ir_module.h"
#include "includes/backend/codegen/ir/ir_templates/ir_function.h"
#include "includes/frontend/parser/ast_extractor.h"
#include "includes/backend/codegen/ir/ir_builder.h"
#include "includes/backend/codegen/ir/ir_templates/ir_base_value.h"
#include "includes/backend/codegen/ir/ir_common_types.h"
#include "includes/shared/common/target_architecture.h"

constexpr int FUNC_IDENTIFIER_INDEX = 0;
constexpr int CONDITION_INDEX = 0;
constexpr int STATEMENT_INDEX = 0;
constexpr int TOKEN_TYPE_INDEX = 0;
constexpr int FOR_PRE_COND_VAR_INDEX = 0;
constexpr int FOR_PRE_COND_ASSIGNMENT_INDEX = 1;
constexpr int FOR_PRE_COND_EXPRESSION_INDEX = 2;
constexpr int FOR_COND_INDEX = 3;
constexpr int FOR_PRE_AFTER_COND_VAR_INDEX = 4;
constexpr int FOR_PRE_AFTER_COND_ASSIGNMENT_INDEX = 5;
constexpr int FOR_PRE_AFTER_COND_EXPRESSION_INDEX = 6;

/// @brief This enum represents the type of entry a control block has done.
/// ifBlock - will create a %then label if it is entered and will create a %else label if not entered.
/// elifBlock - will create a %then label if it is entered and will create a %else label if not entered.
/// elseBlock - will only create a %merge label (EntranceKind::exit) this is the block that will be entered after the else.
/// functionBlock - will only create an entry with the label entry.
/// whileBlock - will first create a condition called while.cond (EntranceKind::condition), a while.body for the code inside (EntranceKind::entry), and a while.exit after the block (EntranceKind::exit)
/// forBlock - will create a condition called for.cond (EntranceKind::condition), a for.body for the code inside (EntranceKind::entry), and a for.exit after the block (EntranceKind::exit)
enum class EntranceKind
{
    entry,
    exit,
    condition,
    latch  // for loops jump here if continue is called
};
enum class BlockKind
{
    functionBlock,
    ifBlock,
    elseBlock,
    elifBlock,
    whileBlock,
    forBlock
};

struct ExprItem
{
    IRBaseValue* value;     // result SSA value
    Operators op;           // operator after this value
    bool signage;
    bool floatness;
    bool isString;          // true if value is string (literal, variable, or string-returning func)
    size_t notCnt;          // how many `not` applied before this value
};

/// @note params of expression func call or sizeof must not be checked the second time through nextChild on the ast
/// @note After usage call DumpConstants() and store somewhere so they wont turn to dangling pointers.
class IRVisitor : public IASTVisitor
{
    public:
    IRVisitor(
        const EntityLogger& entityLogger,
        ErrorManager& errorManager,
        const DecoratedAST& decoratedAst,
        IRModule& module
    );
    virtual ~IRVisitor() = default;

    // Prevent copying/moving - lambdas capture 'this' pointer
    IRVisitor(const IASTVisitor&) = delete;
    IRVisitor& operator=(const IASTVisitor&) = delete;
    IRVisitor(IASTVisitor&&) = delete;
    IRVisitor& operator=(IASTVisitor&&) = delete;

    /// @brief The func dumps all the constants
    /// @note Must be called after finishing IR building!
    std::vector<std::unique_ptr<IRBaseValue>> DumpConstants() noexcept;

    void VisitVarDeclaration(ASTNode* node) override;
    void VisitExpression(ASTNode* node) override;
    void VisitIdentifier(ASTNode* node) override;
    void VisitIdentifierValue(ASTNode* node) override;
    void VisitAssignment(ASTNode* node) override;
    void VisitFuncDeclaration(ASTNode* node) override;
    void VisitFuncDefinition(ASTNode* node) override;
    void VisitFuncCall(ASTNode* node) override;
    void VisitIf(ASTNode* node) override;
    void VisitElif(ASTNode* node) override;
    void VisitElse(ASTNode* node) override;
    void VisitWhile(ASTNode* node) override;
    void VisitFor(ASTNode* node) override;
    void VisitReturn(ASTNode* node) override;
    void VisitControlStatement(ASTNode* node) override;
    void VisitBlockStart(ASTNode* node) override;
    void VisitBlockEnd(ASTNode* node) override;
    void VisitEndOfCommand(ASTNode* node) override;
    void VisitSizeof(ASTNode* node) override;
    void VisitVoidCommand(ASTNode* node) override;
    void VisitImportStatement(ASTNode* node) override;
    void VisitPass(ASTNode* node) override;
    void VisitEof(ASTNode* node) override;
    void VisitArrayAccess(ASTNode* node) override;

private:
    /**********************\
    |        Utils         |
    \**********************/
    const EntityLogger& m_entityLogger;
    ErrorManager& m_errorManager;

    /**********************\
    |     Decorated AST    |
    \**********************/
    const DecoratedAST& m_decoratedAst;
    const VarTableNode* m_currVarTableNode;

    /**********************\
    |   Curr IRTemplates   |
    \**********************/
    IRModule& m_currModule;
    IRFunction* m_currFunc;
    
    /**********************\
    |      IRBuilder       |
    \**********************/
    IRBuilder m_irbuilder;

    /**********************\
    |   Labeling Counters  |
    \**********************/
    size_t m_thenCnt;
    size_t m_elseCnt;
    size_t m_condCnt;
    size_t m_bodyCnt;
    size_t m_exitCnt;
    size_t m_mergeCnt;
    size_t m_latchCnt;

    /**********************\
    |  Control Flow Fields |
    \**********************/

    /// @brief Saves the about to be entry block that is initialized on the next node '{' node.
    std::unique_ptr<BasicBlock> m_nextControlEntryBlock;
    /// @brief Saves the for node for all concurrent loops so that we will be able to add the increment before looop.body close.
    std::stack<ASTNode*> m_forIncrements;
    /// @brief Saves the history of what BlockKind we are in right now.
    std::stack<BlockKind> m_blockHistory;
    /// @brief Saves the loop.exit block so that we will be able to jump to them with 'break'.
    std::stack<BasicBlock*> m_afterLoopBlocks;
    /// @brief Saves the loop.cond blocks so that continue and end of loop.body will jump to them.
    std::stack<BasicBlock*> m_conditionBlocks;
    /// @brief Saves the merge block of a control statement row e.g. if elif elif will all jump to merge after completion.
    std::stack<std::unique_ptr<BasicBlock>> m_conditionMergeBlocks;
    /// @brief Saves the next exit block that is initialized at next '}' node.
    std::stack<std::unique_ptr<BasicBlock>> m_nextExitBlocks;
    /// @brief Target block for 'continue': for-loops latch (increment), while-loops, condition.
    std::stack<BasicBlock*> m_continueTargets;
    /// @brief For-loop latch blocks (hold the increment code); created in VisitFor, added to function in VisitBlockEnd.
    std::stack<std::unique_ptr<BasicBlock>> m_forLatchBlocks;
    /// @brief The current index of the varTable.
    std::stack<size_t> m_varTableIndex;

    /**********************\
    |     Helper Fields    |
    \**********************/
    // e.g. if we just finished an expression after return and need to return to
    // the return we save the expression's value here.
    IRBaseValue* m_lastSavedValue;
    // The var we might assign to.
    std::optional<VarMetaData> m_currVarMetaData;

    bool m_expectedReturnValue;
    bool m_expectedAssignment;

    bool m_indexedAssignment;
    bool m_slicedAssignment;
    bool m_lastExprWasString;
    Command m_pendingIndexCommand;

    bool m_returnedAlready;
    /// @brief This is used to flag that we already handled the insides:
    /// e.g. let's say we had an if statement we had to run the expression inside it at the VisitIf
    /// this means we dont need to run it once again until the curr FullCommand is finished.
    bool m_handledInsides;
    bool m_insideFor;

    /**********************\
    |    Helper Methods    |
    \**********************/
    /// @brief The func transforms a normal Type into an IRType object.
    std::shared_ptr<const IRType> TypeToIRType(Type type);
    std::shared_ptr<const IRType> TypeToIRType(const std::string& type);

    /// @brief The func transforms a const value (e.g. 1, -3.4 true) into an IRConstValue
    /// @param token the const value that is tranformed
    /// @return pointer to the IRConstValue
    IRBaseValue* ValueToConstIR(Token token);

    /// @brief The func is used to choose which operation it should run through IRBuilder
    /// @param usedOperator The operation is chosen by the operator.
    /// @param signage True-> Signed operation. False-> Otherwise.
    /// @param floatness True-> Float operation. False-> Otherwise.
    /// @return The finished operation.
    IRInstruction* ChooseOperation(Operators usedOperator, IRBaseValue* lsh, IRBaseValue* rsh, bool signage, bool floatness);

    /// @brief The func transforms a tokenizer operator into an IROperator.
    /// @param token The operation (includes both operators and keyword operators).
    /// @return The operation as an Operators enum.
    Operators TokenizerOperatorToIROperator(Token token);

    /// @brief Checks if the Token is a constant unsigned value.
    /// @param token A value Token.
    /// @return True-> Token is unsigned. False-> Otherwise.
    bool CheckSignage(Token token);

    /// @brief Checks if the Token is a float.
    /// @param token A value Token.
    /// @return True-> Token is float. False-> Otherwise.
    bool CheckFloatness(Token token);

    /// @brief Creates the related Load Instructions to the current block.
    /// @param node The node that holds the Command that holds the PicoC IdentifierValue.
    /// @return The final Instruction that holds the value of the Load.
    IRBaseValue* HandleIdentifierValue(ASTNode* node);

    /// @brief Creates the related FuncCall Instructions to the current block.
    /// @param command The Command that holds the PicoC FuncCall.
    /// @return The final Instruction that holds the value of the FuncCall.
    IRBaseValue* HandleFuncCall(Command command);

    /// @brief Creates the related Sizeof Instructions to the current block.
    /// @param command The Command that holds the PicoC Sizeof.
    /// @return The final Instruction that holds the value of the Sizeof.
    IRBaseValue* HandleSizeof(Command command);

    /// @brief Parses an expression into SSA Instructions and adds the to the current block.
    /// @param command The Command that holds the PicoC EXPRESSION.
    /// @return The IRBaseValue that holds the last value of the expression.
    IRBaseValue* HandleExpression(Command command);

    /// @brief Handles string index access str[i] by lowering to char_at(str, i).
    IRBaseValue* HandleArrayAccess(const Command& command);

    /// @brief Handles string slice access str[a:b] by lowering to substr(str, a, b-a).
    IRBaseValue* HandleArraySlice(const Command& command);

    /// @brief The func creates a corresponding labelName for each nameless block.
    /// @see EntranceKind for more info
    std::string GetCorrespondingLabel(BlockKind blockKind, EntranceKind entryKind);

    /// @brief A function that is used to check if after we got the value from here we need to give it to somewhoere.
    /// e.g. return of assignment.
    void CheckValueNeededSomewhere(IRBaseValue* value, ASTNode* node);

    /// @brief The func is used to get the last instruction added to the current block.
    /// @return The last instruction in the current block.
    inline IRInstruction* GetLastIsntruction();

    /// @brief The func finds the pointer to a local var.
    /// @param name The name of the var.
    /// @return The StoreInstruction that holds the pointer.
    IRBaseValue* FindPointer(const std::string& name);
    
    void StoreInternalError(const std::string& msg , const Command& command);

    /**********************\
    |   Expression Utils   |
    \**********************/

    /// @brief Helper that checks whether `op` is one of the operators in `set`.
    /// @param op The operator to test.
    /// @param set A small initializer_list of Operators to test membership against.
    /// @return True if op is contained in set, otherwise false.
    static bool IsOneOf(Operators op, std::initializer_list<Operators> set);

    /// @brief Apply 'not' semantics to an IR value according to notCnt.
    /// @param value The IR value to apply 'not' to.
    /// @param notCnt Number of consecutive 'not' tokens seen immediately before value.
    /// @return New IRBaseValue* after applying the nots, or nullptr on error.
    IRBaseValue* ApplyNotIfNeeded(IRBaseValue* value, size_t notCnt);

    /// @brief Insert an implicit FPToSI or SIToFP conversion when the value
    ///        type doesn't match the target variable type.
    IRBaseValue* ConvertIfNeeded(IRBaseValue* value, std::shared_ptr<const IRType> targetType);

    /// @brief Collect tokens from a command into a flat vector of ExprItem.
    /// @details This routine handles parentheses recursively: when a '(' is
    ///          encountered it will recurse and return a single ExprItem that
    ///          represents the entire sub-expression. It also consumes HAS_PARAM
    ///          tokens (function-call params / sizeof) by advancing paramIndex.
    /// @param command The Command that owns tokens and params (used to resolve HAS_PARAM).
    /// @param tokens The token vector (usually command.GetTokens()) to parse.
    /// @param tokenIndex Index into `tokens` which this function advances as it consumes tokens.
    /// @param paramIndex Index into command.GetParams(), advanced when a HAS_PARAM is consumed.
    /// @return A vector of ExprItem representing the flattened expression for this call.
    std::vector<ExprItem> CollectExprItems(
        const Command& command,
        const std::vector<Token>& tokens,
        size_t& tokenIndex,
        size_t& paramIndex
    );

    /// @brief Reduce a vector of ExprItem by a single operator precedence group.
    /// @details Scans left-to-right and collapses occurrences of operators contained
    ///          in `ops` using ChooseOperation. The function modifies `expr` in-place.
    /// @param expr The expression item vector to reduce (modified in-place).
    /// @param ops The operator group handled by this pass (e.g. {Mul,Div,Rem}).
    void ReduceExprByOperators(
        std::vector<ExprItem>& expr,
        std::initializer_list<Operators> ops
    );

    /// @brief Reduce expression by full precedence order.
    /// @details Calls ReduceExprByOperators for each precedence tier from highest
    ///          to lowest (e.g. Mul/Div/Rem -> Add/Sub -> relational -> equality -> bitwise).
    /// @param expr The expression item vector to reduce (modified in-place).
    void ReduceExprByPrecedence(std::vector<ExprItem>& expr);
};
