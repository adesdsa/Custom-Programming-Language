#include "includes/semantic_analysis/semantic_analysis_visitor.h"

SemanticAnalysisVisitor::SemanticAnalysisVisitor() :
    m_errorManager(ErrorManager::GetInstance())
{
    this->InitializeHandlers();
}

void SemanticAnalysisVisitor::VisitVarDeclaration(ASTNode* node)
{
    const std::string& varName = ASTExtractor::ExtractVarName(node);
    Type varType = ASTExtractor::ExtractVarType(node);
    AccessRights varAccessRight = ASTExtractor::ExtractVarAccessRight(node);
    Scope varScope = ASTExtractor::ExtractVarScope(node);

    // FIXME: sketch:
    // VarMetadata var = this->m_varFactory(varName, varType, varAccessRight, varScope);
    // this->m_facade.   => call facade tests on `var`
}

void SemanticAnalysisVisitor::VisitExpression(ASTNode* node)
{
    
}
