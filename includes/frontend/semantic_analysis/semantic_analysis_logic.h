#pragma once

#include "includes/frontend/parser/ast.h"
#include "includes/frontend/semantic_analysis/func_table.h"
#include "includes/frontend/semantic_analysis/var_table_node.h"
#include "includes/frontend/semantic_analysis/ast_iterator.h"
#include "includes/frontend/semantic_analysis/semantic_analysis_visitor.h"

struct DecoratedAST
{
    const AST& ast;
    FuncTable funcTable;
    VarTableNode varTable;

    DecoratedAST(const AST& ast, const EntityLogger& semanticAnalysisLogger) :
        ast(ast),
        funcTable(semanticAnalysisLogger),
        varTable(nullptr, semanticAnalysisLogger)
    {}
};

class SemanticAnalysisLogicComponent
{
public:
    SemanticAnalysisLogicComponent(const AST& ast);
    ~SemanticAnalysisLogicComponent() = default;

    /*****************************************************************************
     * @brief Runs Semantic Analysis Pipeline:                                   |
     *                                                                           |
     * This method performs the complete semantic analysis process:              |
     *                                                                           |
     * 1. Iterating through AST nodes using the ASTIterator.                     |
     *   @note: Each node may represent a function declaration/definition,       |
     *          variable declaration, condition statement, etc....               |
     *                                                                           |
     *   1.1. Each AST node goes through SemanticAnalysisVisitor.                |
     *                                                                           |
     *   1.2. If node passes semantic validation (via SemanticCheckerFacade),    |
     *        populate symbol tables (FuncTable, VarTableNode).                  |
     *                                                                           |
     *   1.3. If node is invalid, semantic errors are recorded via ErrorManager. |
     *                                                                           |
     * 2. Repeats process from step 1 until all AST nodes are visited.           |
     *                                                                           |
     * 3. Verifies all declared functions have definitions.                      |
     *                                                                           |
     * 4. If any errors occured, handle them and abort compilation.              |
     *                                                                           |
     * 5. Otherwise, outputs the DecoratedAST (AST + symbol tables).             |
     *                                                                           |
     * @see ASTIterator                                                          |
     * @see SemanticAnalysisVisitor                                              |
     * @see SemanticCheckerFacade                                                |
     * @see DecoratedAST                                                         |
     ****************************************************************************/
    const DecoratedAST& MainFlow();

private:
    EntityLogger m_semanticAnalysisLogger;
    ErrorManager& m_errorManager;

    ASTIterator m_astIterator;
    DecoratedAST m_decoratedAST;
    SemanticAnalysisVisitor m_semanticAnalysisVisitor;

    /// @brief Check that all declared functions have matching defination
    void HandleUndefinedFunctions();
};
