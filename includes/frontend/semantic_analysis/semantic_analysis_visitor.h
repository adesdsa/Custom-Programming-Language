#pragma once

#include <stack>

#include "includes/shared/common/abstract_ast_visitor.h"
#include "includes/shared/errors/error_manager.h"
#include "includes/frontend/parser/ast_extractor.h"
#include "includes/frontend/semantic_analysis/var_factory.h"
#include "includes/frontend/semantic_analysis/func_factory.h"
#include "includes/frontend/semantic_analysis/semantic_checker_facade.h"
#include "includes/frontend/semantic_analysis/var_metadata.h"

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
    VarTableNode* m_functionScopeRoot = nullptr;
    ExpressionType m_lastExpressionType;

    std::string m_lastIdentifierName;
    bool m_identifierError = false;
    
    std::string m_currentFunctionName;

    bool m_bStartedFuncDefinition; // If we are in a func definition we open the block in the 
    // varTable before we get to the '{' Token inorder to save the arguments.
    bool m_bStartedFuncDeclaration; // If we are in a func declaration we dont save the var's yet.

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

    // stack to track which blocks need an extra scope pop (for-loop init scope)
    std::stack<bool> m_forInitScopeStack;
    bool m_pendingForInitScope = false;

    // pending for loop condition check, consumed by VisitExpression
    // The condition can't be checked in VisitFor because the loop variable isn't in scope yet
    bool m_pendingForConditionCheck = false;
    size_t m_forConditionLine = 0;
    size_t m_forConditionColumn = 0;
    int m_forExpressionCount = 0;  // 0=init value, 1=condition, 2+=increment

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

    /// @brief The func is used to handle single Command assignments
    /// which means either identifier_value or finished_func_call
    /// @param node `ASTNode*`: node that passed as agrument to function call  
    void HandleSingleCommandAssignement(ASTNode* node);
};
