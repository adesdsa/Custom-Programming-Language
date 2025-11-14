#include "../../includes/parser_component/ast_node.h"

ASTNode::ASTNode(const Token& token) : 
    m_token(token), 
    m_nodeFirstChild(nullptr), 
    m_nodeNextSibling(nullptr) {}

void ASTNode::StoreFullCommand(ASTArena& arena, std::vector<Token>& fullCommand)
    {
        if (fullCommand.empty()) return;

        ASTNode* prevChild = nullptr;

        for (auto& tok : fullCommand)
        {
            ASTNode* newNode = arena.AllocateNode(tok);

            if (!m_nodeFirstChild)
                m_nodeFirstChild = newNode;
            else
                prevChild->m_nodeNextSibling = newNode;

            prevChild = newNode;
        }
    }