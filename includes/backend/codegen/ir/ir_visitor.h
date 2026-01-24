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
/// elseBlock - will only create a %then label (EntranceKind::entry) this is the block that will be entered after the else.
/// functionBlock - will only create an entry with the label entry.
/// whileBlock - will create an entry called while.cond (EntranceKind::condition), a while.body for the code inside (EntranceKind::entry), and a while.exit after the block (EntranceKind::exit)
/// forBlock - will create an entry called for.cond (EntranceKind::condition), a for.body for the code inside (EntranceKind::entry), and a for.exit after the block (EntranceKind::exit)
enum class EntranceKind
{
    entry,
    exit,
    condition
};

/// @todo check that code after Br isnt added

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

    /**********************\
    |     Helper Fields    |
    \**********************/
    IRBaseValue* m_lastSavedValue;
    std::optional<VarMetaData> m_currVarMetaData;

    bool m_expectedReturnValue;
    bool m_expectedAssignment;

    std::stack<BlockKind> m_blockHistory;
    std::stack<std::unique_ptr<BasicBlock>> m_afterLoopBlocks;
    std::stack<std::vector<IRBaseValue*>> m_funcArgs;

    /**********************\
    |    Helper Methods    |
    \**********************/
    std::shared_ptr<const IRType> TypeToIRType(Type type);
    IRBaseValue* ValueToConstIR(Token token);
    IRInstruction* ChooseOperation(Operators usedOperator, IRBaseValue* lsh, IRBaseValue* rsh, bool signage, bool floatness);
    Operators TokenizerOperatorToIROperator(Token token);
    bool CheckSignage(Token token);
    bool CheckFloatness(Token token);
    /// @brief Adds all the related IRInstructions to var load
    /// @param command 
    /// @return The value of the variable
    IRBaseValue* HandleIdentifierValue(ASTNode* node);
    /// @brief Adds all the related IRInstructions to func call
    /// @param command 
    /// @return The value of the variable
    IRBaseValue* HandleFuncCall(Command command);
    /// @brief Adds all the related IRInstructions to func call
    /// @param command 
    /// @return The value of the variable
    IRBaseValue* HandleSizeof(Command command);
    /// @brief Parses an expression into the last value it is worth and adds the corresponding instruction
    /// @param command 
    /// @return The value the expression creates
    IRBaseValue* HandleExpression(Command command);
    IRBaseValue* HandleExpressionToInstruction(IRBaseValue* oldInstruction, IRBaseValue* newInstruction, size_t notCnt, Operators lastOperator, bool signage, bool floatness);
    std::string GetCorrespondingLabel(BlockKind blockKind, EntranceKind entryKind);
    void CheckValueNeededSomewhere(IRBaseValue* value, ASTNode* node);
};
