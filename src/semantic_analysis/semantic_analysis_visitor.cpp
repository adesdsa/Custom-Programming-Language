#include "includes/semantic_analysis/semantic_analysis_visitor.h"

SemanticAnalysisVisitor::SemanticAnalysisVisitor(const EntityLogger& entityLogger) :
    m_entityLogger(entityLogger),
    m_errorManager(ErrorManager::GetInstance()),
    m_varFactory(entityLogger)
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
    VarMetaData var = this->m_varFactory.CreateVarMetadata(
        varName, 
        varType, 
        varAccessRight, 
        varScope
    );

    // this->m_facade.   => call facade tests on `var`
}

void SemanticAnalysisVisitor::VisitExpression(ASTNode* node)
{

}
