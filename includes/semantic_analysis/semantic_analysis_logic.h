#pragma once

#include "includes/parser/ast.h"
#include "includes/semantic_analysis/func_table.h"
#include "includes/semantic_analysis/var_table_node.h"

struct DecoratedAST
{
    const AST& ast;
    FuncTable funcTable;
    VarTableNode varTable;
};

class SemanticAnalysisLogicComponent
{
public:
    SemanticAnalysisLogicComponent(const AST& ast);
    ~SemanticAnalysisLogicComponent() = default;

    // TODO: Fix docs of the MainFlow
    /*****************************************************************************
     * @brief Runs Semantic Analysis Pipeline:                                   |
     *                                                                           |
     * This method performs the complete semantic analysis process:              |
     *                                                                           |
     * 1. Building FullCommand objects by fetching tokens on demand.             |
     *  @note: A full command may represent a function declaration,              |
     *          variable declaration, condition statement, etc....               |
     *                                                                           |
     * 2. Each full command goes through SyntaxChecker.                          |
     *                                                                           |
     * 3. If the command passes syntax validation, insert into the AST           |
     *      as a new subtree.                                                    |
     *                                                                           |
     * 4. If command is invalid, syntax errors are recorded via error manager    |                                   |
     *                                                                           |
     * 5. Repeats proccess from step 1 until the source input is fully consumed  |
     *                                                                           |
     * 6. If any errors occured, handle them and abort compilation               |
     *                                                                           |
     * 7. Otherwise, outputs the built AST                                       |
     *                                                                           |
     * @see Command                                                              |
     * @see FullCommand                                                          |
     * @see AST                                                                  |
     ****************************************************************************/
    const DecoratedAST& MainFlow();

private:
    DecoratedAST m_decoratedAST;

    const EntityLogger& m_semanticAnalysisLogger;
};
