#include "includes/backend/codegen/ir/ir_generator.h"

IRGenerator::IRGenerator(const DecoratedAST& decoratedAST) :
    m_IRLogger("IR"),
    m_errorManager(ErrorManager::GetInstance()),
    m_astIterator(decoratedAST.ast),
    m_irMoudle("program") // TODO: put the output file name instead
{}

const IRModule& IRGenerator::MainFlow()
{
    return this->m_irMoudle;
}
