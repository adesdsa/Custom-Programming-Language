#pragma once

#include "includes/frontend/parser/ast.h"
#include "includes/frontend/semantic_analysis/func_table.h"
#include "includes/frontend/semantic_analysis/var_table_node.h"

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