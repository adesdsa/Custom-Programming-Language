#pragma once

#include "includes/shared/common/abstract_ast_visitor.h"
#include "includes/shared/common/ast_helpers/decorated_ast.h"
#include "includes/shared/errors/error_manager.h"
#include "includes/backend/codegen/ir/ir_builder.h"
#include "includes/backend/codegen/ir/ir_templates/ir_module.h"
#include "includes/backend/codegen/ir/ir_templates/ir_function.h"
#include "includes/frontend/parser/ast_extractor.h"
#include "includes/backend/codegen/ir/ir_builder.h"


/// @todo check that code after Br isnt added

class IRVisitor : public IASTVisitor
{
    public:
    IRVisitor();
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
    IRFunction& m_currFunc;
    
    /**********************\
    |      IRBuilder       |
    \**********************/
    IRBuilder m_irbuilder;

    /**********************\
    |     Helper Fields    |
    \**********************/
    IRInstruction* m_lastSavedInstruction;
    const VarMetaData& m_currVarMetaData;

    /**********************\
    |    Helper Methods    |
    \**********************/
    IRType TypeToIRType(Type type);
}
