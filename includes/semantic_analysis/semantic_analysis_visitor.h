#pragma once

#include "includes/common/abstract_ast_visitor.h"
#include "includes/errors/error_manager.h"
#include "includes/parser/ast_extractor.h"

#include "includes/semantic_analysis/var_metadata.h"

/**
 * @brief Responsible for combining all semantic analysis logic all together:
 * 
 * Contains all handler for each command:
 *    1. Deconstruct command
 *    2. Create corresponding metadata (if needed) based on step 1 result
 *    3. Call facade on the metadata (if exist) or result of step 1
 *    4. IF: Facade passed -> Insert into the corresponding field in 
 *          DecoratedAST the object (if exist)
 *    5. ELSE: Store corresponding logic errors in errorManager
 * 
 * @see SEMANTIC_ANALYSIS.md
*/
class SemanticAnalysisVisitor : public IASTVisitor
{
public:
    SemanticAnalysisVisitor();
    ~SemanticAnalysisVisitor() = default;

    virtual void VisitVarDeclaration(ASTNode* node) override;
    virtual void VisitExpression(ASTNode* node) override;

private:
    ErrorManager& m_errorManager;
};
