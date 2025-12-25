#include "includes/semantic_analysis/semantic_analysis_logic.h"

SemanticAnalysisLogicComponent::SemanticAnalysisLogicComponent(const AST& ast) :
    m_semanticAnalysisLogger(k_semanticAnalysisLoggerName),
    m_decoratedAST(ast, this->m_semanticAnalysisLogger)
{
    this->m_semanticAnalysisLogger.Info(
        "Finished initializing Semantic Analysis component successfully"
    );
}