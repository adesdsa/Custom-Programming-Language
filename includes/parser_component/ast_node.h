#pragma once

#include "../tokenizer/token.h"
// #include <cstddef>

// Left-child, right-sibling AST node
// Each node contains full command (function, variable decleraion  etc...)
class ASTNode
{
public:
    ASTNode(const Token& token);
    ~ASTNode() = default;

    // Adds a command subtree from fullCommand tokens
    void StoreFullCommand(AST& ast, std::vector<Token>& fullCommand);

    Token m_token;
    ASTNode* m_nodeFirstChild;
    ASTNode* m_nodeNextSibling;
};
