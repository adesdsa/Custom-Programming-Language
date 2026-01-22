#include "includes/backend/codegen/ir/ir_generator.h"

IRGenerator::IRGenerator(const DecoratedAST& decoratedAST) :
    m_IRLogger("IR"),
    m_errorManager(ErrorManager::GetInstance()),
    m_astIterator(decoratedAST.ast),
    m_irMoudle("program") // TODO: put the output file name instead
{
    this->m_IRLogger.Info("Finished initializing IR Generator component successfully");
}

const IRModule& IRGenerator::MainFlow()
{
    while (this->m_astIterator.HasNext())
    {
        ASTNode* curr = this->m_astIterator.GetCurrent();
        // this->m_irVisitor.Visit(curr);
        (void)this->m_astIterator.Next();
    }

    if (this->m_errorManager.HasErrorsOccured())
    {
        this->m_errorManager.HandleErrors();
    }

    this->m_IRLogger.Info("Finished generating IR");
    return this->m_irMoudle;
}
