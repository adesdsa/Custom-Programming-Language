#pragma once

#include <stack>

#include "includes/common/abstract_ast_visitor.h"
#include "includes/errors/error_manager.h"
#include "includes/parser/ast_extractor.h"
#include "includes/semantic_analysis/var_factory.h"
#include "includes/semantic_analysis/func_factory.h"
#include "includes/semantic_analysis/semantic_checker_facade.h"
#include "includes/semantic_analysis/var_metadata.h"

constexpr int IDENTIFIER_NAME_INDEX = 0;
constexpr int FIRST_TOKEN_INDEX = 0; 
constexpr int FUNCTION_NAME_TOKEN_INDEX = 0;
constexpr int VAR_NAME_TOKEN_INDEX = 0;
constexpr int CONDITION_TOKEN_INDEX = 0;
constexpr int STATEMENT_TOKEN_INDEX = 0;

class SemanticAnalysisVisitor : public IASTVisitor
{
public:
    SemanticAnalysisVisitor(
        const EntityLogger& entityLogger, 
        VarTableNode& varTable,
        FuncTable& functable
    );
    ~SemanticAnalysisVisitor() = default;

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
    VarTableNode& m_varTable;
    FuncTable& m_funcTable;

    /**********************\
    |      Factories       |
    \**********************/
    VarFactory m_varFactory;
    FuncFactory m_funcFactory;

    /**********************\
    |     Helper Fields    |
    \**********************/
    VarTableNode* m_currentScope;
    ExpressionType m_lastExpressionType;

    std::string m_lastIdentifierName;
    bool m_identifierError = false;
    
    std::string m_currentFunctionName;

    // pending assignment tracking, consumed by VisitExpression
    bool m_pendingAssignment = false;
    size_t m_assignmentLine = 0;
    size_t m_assignmentColumn = 0;

    // pending block tracking, consumed by VisitBlockStart
    SemanticCheckerFacade::BlockKind m_pendingBlockKind;
    bool m_hasPendingBlock = false;
    
    // stack to track which blocks called EnteredBlock and need ExitedBlock
    // Each bool represents: did this block level call EnteredBlock?
    std::stack<bool> m_blockEnteredStack;

    /**********************\
    |    Helper Methods    |
    \**********************/

    /// @brief IF & ELSE IF & ELSE all have same checks, this will handle all
    void VisitIfOrElseIfOrWhile(ASTNode* node);

    /// @brief If some logic errors already detected in handler
        /// then next handlers in the chain may need use this knowlage
    void SignalFailureForNextHandlers() noexcept;

    /// @brief Clears all state related to the deferred assignment type check mechanism.
    /// 
    /// Called when:
    ///   - Assignment completes (after type check in VisitExpression)
    ///   - Assignment fails (invalid/const/readonly identifier)
    ///   - Guards trigger (null node, empty tokens)
    ///
    /// Resets:
    ///   - m_lastIdentifierName: Variable name waiting for type check
    ///   - m_pendingAssignment:  Flag for VisitExpression to do type check
    ///   - m_assignmentLine/Col: Location of '=' for error messages
    ///   - m_identifierError:    Flag indicating VisitIdentifier found error
    void ResetAssignmentState() noexcept;

    /// @brief  Converts `Type` enum to string. For instance: BOOL_TYPE will become "bool"
    /// @param type `Type enum`: Type to convert to string
    /// @return `std::string`: Type converted in string format.
    std::string TypeToString(Type type) const noexcept;

    /// @brief  Converts `ExpressionType` enum to string.
    /// @param expressionType `ExpressionType enum`: Type to convert to string
    /// @return `std::string`: Type converted in string format.
    std::string ExpressionTypeToString(ExpressionType expressionType) const noexcept;

    /// @brief Evaluate given node into an `ExpressionType enum`
    /// @param node `ASTNode*`: node that passed as agrument to function call 
    /// @return `ExpressionType enum`: infered type of the node's expression
    ExpressionType EvaluateNodeType(ASTNode* node) const noexcept;

    /// @brief Following methods cover facade checks that are shared between Visits ||
    //                                                                              \/

    /// @brief Calls SemanticCheckerFacade::EnteredBlock for the given block kind.
    /// @return true if EnteredBlock was called, false otherwise (e.g., function not found)
    bool HandleEnteredBlockCheck(
        ASTNode* node, 
        SemanticCheckerFacade::BlockKind blockkind
    );

    void HandleExitedBlockCheck(ASTNode* node) const;
};
