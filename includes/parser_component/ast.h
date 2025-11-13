#pragma once

#include "../tokenizer/token.h"

// Left-child right-sibling ast representation
class ASTNode
{
public:
    ASTNode(const Token& token);

private:
    Token m_token;
    ASTNode* m_nodeFirstChild;
    ASTNode* m_nodeNextSibling;
};