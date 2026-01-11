#include "includes/semantic_analysis/semantic_analysis_logic.h"

SemanticAnalysisLogicComponent::SemanticAnalysisLogicComponent(const AST& ast) :
    m_semanticAnalysisLogger("SEMANTIC_ANALYSIS"),
    m_astIterator(ast),
    m_decoratedAST(ast, this->m_semanticAnalysisLogger)
{
    this->m_semanticAnalysisLogger.Info(
        "Finished initializing Semantic Analysis component successfully"
    );
}

const DecoratedAST& SemanticAnalysisLogicComponent::MainFlow()
{
    while (this->m_astIterator.HasNext())
    {
        ASTNode* curr = this->m_astIterator.GetCurrent();
        (void)this->m_astIterator.Next();
    }

    return this->m_decoratedAST;
}
