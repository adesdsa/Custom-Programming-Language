#include "../../includes/parser_component/ast_node.h"

ASTNode::ASTNode(const Token& token) : 
    m_token(token), 
    m_nodeFirstChild(nullptr), 
    m_nodeNextSibling(nullptr) {}

void ASTNode::StoreFullCommand(AST& ast, std::vector<Token>& fullCommand)
{
    if (fullCommand.empty()) return;

    // keeps track of prev node to than link the next sibling with it
    ASTNode* prevChild = nullptr;

    for (auto& token : fullCommand)
    {
        ASTNode* newNode = ast.AllocateNode(token);

        if (!this->m_nodeFirstChild)
        {
            this->m_nodeFirstChild = newNode;
        }
        else
        {
            prevChild->m_nodeNextSibling = newNode;
        }
        prevChild = newNode;
    }
}