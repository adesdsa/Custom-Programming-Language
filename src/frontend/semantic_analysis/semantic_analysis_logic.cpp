#include "includes/frontend/semantic_analysis/semantic_analysis_logic.h"
#include "includes/frontend/semantic_analysis/semantic_checker_facade.h"
#include "includes/frontend/parser/ast_extractor.h"
#include "includes/shared/errors/error_manager.h"

SemanticAnalysisLogicComponent::SemanticAnalysisLogicComponent(const AST& ast, const ProjectSymbolTable& projectSymbolTable) :
    m_semanticAnalysisLogger("SEMANTIC_ANALYSIS"),
    m_errorManager(ErrorManager::GetInstance()),
    m_astIterator(ast),
    m_decoratedAST(ast, this->m_semanticAnalysisLogger),
    m_semanticAnalysisVisitor(
        this->m_semanticAnalysisLogger,
        this->m_decoratedAST.varTable,
        this->m_decoratedAST.funcTable,
        projectSymbolTable
    )
{
    this->m_semanticAnalysisLogger.Info(
        "Finished initializing Semantic Analysis component successfully"
    );
}

const DecoratedAST& SemanticAnalysisLogicComponent::MainFlow()
{
    ErrorManager::SetCurrentReporter("SEMANTIC_ANALYSIS");
    this->CollectGlobalVariables();

    while (this->m_astIterator.HasNext())
    {
        ASTNode* curr = this->m_astIterator.GetCurrent();
        this->m_semanticAnalysisVisitor.Visit(curr);
        (void)this->m_astIterator.Next();
    }
    this->HandleUndefinedFunctions();

    if (this->m_errorManager.HasErrorsOccured())
    {
        this->m_errorManager.HandleErrors();
    }

    this->m_semanticAnalysisLogger.Info("Finished semantically analyze file");
    return this->m_decoratedAST;
}

void SemanticAnalysisLogicComponent::CollectGlobalVariables()
{
    ASTIterator prePass(this->m_decoratedAST.ast);
    VarFactory varFactory(this->m_semanticAnalysisLogger);

    while (prePass.HasNext())
    {
        ASTNode* node = prePass.GetCurrent();

        if (node && node->command.GetType() == CommandTypes::VAR_DECLARATION)
        {
            Scope scope = ASTExtractor::ExtractVarScope(node);
            if (scope == Scope::GLOBAL || scope == Scope::EXPORT)
            {
                const std::string& varName = ASTExtractor::ExtractVarName(node);

                if (!this->m_decoratedAST.varTable.FindVar(varName))
                {
                    Type varType = ASTExtractor::ExtractVarType(node);
                    AccessRights varAccess = ASTExtractor::ExtractVarAccessRight(node);

                    VarMetaData var = varFactory.CreateVarMetadata(
                        varName, varType, varAccess, Scope::GLOBAL
                    );
                    this->m_decoratedAST.varTable.InsertVar(var);
                }
            }
        }

        (void)prePass.Next();
    }
}

void SemanticAnalysisLogicComponent::HandleUndefinedFunctions()
{
    SemanticCheckerFacade tempFacade;
    const std::vector<FuncMetaData*> undefinedFunctions = tempFacade.AreAllFunctionsDefined(
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
}
