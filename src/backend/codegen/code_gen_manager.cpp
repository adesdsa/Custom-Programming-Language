#include "includes/backend/codegen/code_gen_manager.h"

CodeGenManager::CodeGenManager(const DecoratedAST& decoratedAST) :
    m_decoratedAST(decoratedAST),
    m_errorManager(ErrorManager::GetInstance())
{}

void CodeGenManager::MainFlow()
{
    IRGenerator irGenerator(this->m_decoratedAST);
    IRModule& irProgram = irGenerator.MainFlow();

    OptimizationManager optimizationManager;
    optimizationManager.MainFlow(irProgram);

    ASMGenerator asmGenerator(irProgram, this->m_decoratedAST.funcTable);
    std::string asmFilePath = asmGenerator.MainFlow();

    //TODO: Insert assembler
}
