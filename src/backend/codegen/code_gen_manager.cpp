#include "includes/backend/codegen/code_gen_manager.h"

CodeGenManager::CodeGenManager(const DecoratedAST& decoratedAST) :
    m_decoratedAST(decoratedAST)
{}

void CodeGenManager::MainFlow()
{
    IRGenerator irGenerator(this->m_decoratedAST);
    (void)irGenerator.MainFlow();
}