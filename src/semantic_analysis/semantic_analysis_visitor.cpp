#include "includes/semantic_analysis/semantic_analysis_visitor.h"

SemanticAnalysisVisitor::SemanticAnalysisVisitor() :
    m_errorManager(ErrorManager::GetInstance())
{
    this->InitializeHandlers();
}

void SemanticAnalysisVisitor::VisitVarDeclaration(ASTNode* node)
{
    
}