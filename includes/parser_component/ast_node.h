#pragma once

#include "parser/commandLine.h"

// Left-child, right-sibling AST node
// Each node contains full command (function, variable decleraion  etc...)
class ASTNode
{
public:
    ASTNode() = default;
    ~ASTNode() = default;

    Command m_command;
    ASTNode* m_nodeFirstChild;
    ASTNode* m_nodeNextSibling;
}; 
