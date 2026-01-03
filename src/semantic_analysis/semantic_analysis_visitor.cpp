#include "includes/semantic_analysis/semantic_analysis_visitor.h"

SemanticAnalysisVisitor::SemanticAnalysisVisitor(
    const EntityLogger& entityLogger,
    VarTableNode& varTable,
    FuncTable& functable
) :
    m_entityLogger(entityLogger),
    m_errorManager(ErrorManager::GetInstance()),
    m_varTable(varTable),
    m_funcTable(functable),
    m_varFactory(entityLogger),
    m_funcFactory(entityLogger),
    m_currentScope(&varTable)  // starts at root scope
{
    this->InitializeHandlers();
}

void SemanticAnalysisVisitor::VisitVarDeclaration(ASTNode* node)
{
    // saves var information from `node->command`
    const std::string& varName = ASTExtractor::ExtractVarName(node);
    Type varType = ASTExtractor::ExtractVarType(node);
    AccessRights varAccessRight = ASTExtractor::ExtractVarAccessRight(node);
    Scope varScope = ASTExtractor::ExtractVarScope(node);

    if (SemanticCheckerFacade::CheckVarExistence(*this->m_currentScope, varName))
    {
        this->m_errorManager.StoreError(
            CompileTimeError(
                ErrorTypes::NAME_ERROR,
                "Variable '" + varName + "' is already declared in this scope",
                node->command.GetTokens()[0].GetLine(),
                node->command.GetTokens()[0].GetColumn(),
                "Check if variable already declared, remember no shadowing allowed!"
            )
        );

        return;
    }

    VarMetaData var = this->m_varFactory.CreateVarMetadata(
        varName, 
        varType, 
        varAccessRight, 
        varScope
    );

    bool inserted = this->m_currentScope->InsertVar(var);

    if (!inserted)
    {
        this->m_errorManager.StoreError(
            CompileTimeError(
                ErrorTypes::INTERNAL_ERROR,
                "Failed to insert variable '" + varName + "'",
                node->command.GetTokens()[0].GetLine(),
                node->command.GetTokens()[0].GetColumn(),
                "Try to recompile file"
            )
        );

        return;
    }
}

void SemanticAnalysisVisitor::VisitExpression(ASTNode* node)
{

}
