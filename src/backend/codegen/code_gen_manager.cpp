#include "includes/backend/codegen/code_gen_manager.h"

CodeGenManager::CodeGenManager(const DecoratedAST& decoratedAST) :
    m_decoratedAST(decoratedAST)
{}

void CodeGenManager::MainFlow()
{
    IRGenerator irGenerator(this->m_decoratedAST);
    const IRModule& irProgram = irGenerator.MainFlow();

    OptimizationManager optimizationManager;
    optimizationManager.MainFlow(irProgram);

    std::cout << "Hello" << std::endl;
}
