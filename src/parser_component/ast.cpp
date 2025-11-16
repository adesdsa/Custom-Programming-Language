#include "../../includes/parser_component/ast.h"

AST::~AST() 
{
    this->Clear();
}

ASTNode* AST::AllocateNode(const Token& token)
{
    this->m_nodes.emplace_back(new ASTNode(token));
    return this->m_nodes.back();
}

void AST::Clear()
{
    for (auto* node : this->m_nodes)
    {
        delete node;
    }
    this->m_nodes.clear();
}