#pragma once

#include "tokenizer/token.h"
// #include <cstddef>

class AST;

// Left-child, right-sibling AST node
// Each node contains full command (function, variable decleraion  etc...)
class ASTNode
{
public:
    ASTNode(const Token& token);
    ~ASTNode() = default;

    // Adds a command subtree from given full command
    // @param ast: The ast that holds this node.
    // @param fullCommand: Vector of tokens that represent command in the program,
    // the vector is sorted from first token in the command to the last one.
    void StoreFullCommand(AST& ast, std::vector<Token>& fullCommand);

    Token m_token;
    ASTNode* m_nodeFirstChild;
    ASTNode* m_nodeNextSibling;
}; 
