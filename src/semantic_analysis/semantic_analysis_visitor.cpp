#include "includes/semantic_analysis/semantic_analysis_visitor.h"

SemanticAnalysisVisitor::SemanticAnalysisVisitor() :
    m_errorManager(ErrorManager::GetInstance())
{
    this->InitializeHandlers()
}

void SemanticAnalysisVisitor::VisitVarDeclaration(ASTNode* node)
{
    // peek at next sibling to see if it an assignment
    ASTNode* nextSibling = node->nodeNextSibling;
    ASTNode* assignmentNode = nullptr;
    ASTNode* valueNode = nullptr;
    
    if (nextSibling && nextSibling->command.GetType() == CommandTypes::ASSIGNMENT)
    {
        assignmentNode = nextSibling;
        
        // Value is next after assignment (EXPRESSION or FUNC_CALL)
        ASTNode* assignedNode = assignmentNode->nodeNextSibling;
        if (assignedNode)
        {
            std::string_view valType = valueCandidate->command.GetType();
            if (valType == CommandTypes::EXPRESSION || 
                valType == CommandTypes::FINISHED_FUNC_CALL)
                {

                }
        }
    }

    /*
    VarMetaData var = m_varFactory.CreateVarMetadata();

    if (!m_semanticFacade.checkVarDecleration(var))
    {
        errorManager.StoreError();
    }

    m.decoratedAST.varTable.Insert(var);
    */
}