#include "includes/semantic_analysis/semantic_analysis_logic.h"
#include "includes/semantic_analysis/semantic_checker_facade.h"
#include "includes/errors/error_manager.h"

SemanticAnalysisLogicComponent::SemanticAnalysisLogicComponent(const AST& ast) :
    m_semanticAnalysisLogger("SEMANTIC_ANALYSIS"),
    m_errorManager(ErrorManager::GetInstance()),
    m_astIterator(ast),
    m_decoratedAST(ast, this->m_semanticAnalysisLogger),
    m_semanticAnalysisVisitor(
        this->m_semanticAnalysisLogger,
        this->m_decoratedAST.varTable,
        this->m_decoratedAST.funcTable
    )
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
        this->m_semanticAnalysisVisitor.Visit(curr);
        (void)this->m_astIterator.Next();
    }

    const std::vector<FuncMetaData*> undefinedFunctions = SemanticCheckerFacade::AreAllFunctionsDefined(
        this->m_decoratedAST.funcTable
    );

    // provide final check for every undefined function
    for (const FuncMetaData* undefinedFunc : undefinedFunctions)
    {
        this->m_errorManager.StoreError(
            CompileTimeError(
                ErrorTypes::LINKING_EROR,
                "Function '" + undefinedFunc->GetName() + "' has been declared but never defined",
                undefinedFunc->GetDeclarationLine(),
                undefinedFunc->GetDeclarationColumn(),
                "Please provide a function definition for function: " + undefinedFunc->GetName()
            )
        );
    }

    if (this->m_errorManager.HasErrorsOccured())
    {
        this->m_errorManager.HandleErrors();
    }

    return this->m_decoratedAST;
}
