#pragma once

#include <stack>

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

enum class BlockKind
{
    functionBlock,
    ifBlock,
    elseBlock,
    elifBlock,
    whileBlock,
    forBlock
};

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
    condition
};

/// @todo check that code after Br isnt added
/// @todo rethink phi

/// @note params of expression func call or sizeof must not be checked the second time through nextChild on the ast
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

protected:
    /**
        ******************************************************************
        * Following methods are handler methods for each type.           *
        * Each child of this abstract class will imploment those methods *
        * based on his needs.                                            *
        ******************************************************************
    */

    virtual void VisitVarDeclaration(ASTNode* node) override;
    virtual void VisitExpression(ASTNode* node) override;
    virtual void VisitIdentifier(ASTNode* node) override;
    virtual void VisitIdentifierValue(ASTNode* node) override;
    virtual void VisitAssignment(ASTNode* node) override;
    virtual void VisitFuncDeclaration(ASTNode* node) override;
    virtual void VisitFuncDefinition(ASTNode* node) override;
    virtual void VisitFuncCall(ASTNode* node) override;
    virtual void VisitIf(ASTNode* node) override;
    virtual void VisitElif(ASTNode* node) override;
    virtual void VisitElse(ASTNode* node) override;
    virtual void VisitWhile(ASTNode* node) override;
    virtual void VisitFor(ASTNode* node) override;
    virtual void VisitReturn(ASTNode* node) override;
    virtual void VisitControlStatement(ASTNode* node) override;
    virtual void VisitBlockStart(ASTNode* node) override;
    virtual void VisitBlockEnd(ASTNode* node) override;
    virtual void VisitEndOfCommand(ASTNode* node) override;
    virtual void VisitSizeof(ASTNode* node) override;
    virtual void VisitVoidCommand(ASTNode* node) override;
    virtual void VisitPass(ASTNode* node) override;
    virtual void VisitEof(ASTNode* node) override;

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

    std::stack<BlockKind> m_blockHistory;
    std::stack<std::unique_ptr<BasicBlock>> m_nextExitBlocks;
    std::unique_ptr<BasicBlock> m_nextControlEntryBlock;
    std::stack<BasicBlock*> m_afterLoopBlocks;
    std::stack<ASTNode*> m_forIncrements;
    std::stack<BasicBlock*> m_conditionBlocks;
    std::stack<std::unique_ptr<BasicBlock>> m_conditionMergeBlocks;
    std::stack<std::vector<IRBaseValue*>> m_funcArgs; // temporary holder to save func arguements
    std::stack<size_t> m_varTableIndex;

    /**********************\
    |    Helper Methods    |
    \**********************/
    /// @brief The func transforms a normal Type into an IRType object.
    std::shared_ptr<const IRType> TypeToIRType(Type type);
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
    /// @brief The func does all the related checks to adding a single SSA expression to the block.
    /// @param oldInstruction The last SSA expression Instruction.
    /// @param newInstruction The Instruction we try adding to it.
    /// @param notCnt The counter of how many times there was not before the newInstruction
    /// @param lastOperator The operation that will happen between them.
    /// @param signage Is the operation unsigned.
    /// @param floatness Is the operation a float.
    /// @return An IRInstruction that holds the value of the operation between the two Instructions.
    IRBaseValue* HandleExpressionToInstruction(IRBaseValue* oldInstruction, IRBaseValue* newInstruction, size_t notCnt, Operators lastOperator, bool signage, bool floatness);
    /// @brief The func creates a corresponding labelName for each nameless block.
    /// @see EntranceKind for more info
    std::string GetCorrespondingLabel(BlockKind blockKind, EntranceKind entryKind);
    /// @brief A function that is used to check if after we got the value from here we need to give it to somewhoere.
    /// e.g. return of assignment.
    void CheckValueNeededSomewhere(IRBaseValue* value, ASTNode* node);
    inline IRInstruction* GetLastIsntruction();
};
