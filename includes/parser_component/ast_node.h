#pragma once

#include "parser/commandLine.h"

class ASTNode
{
public:
    ASTNode() = default;
    explicit ASTNode(const Command& cmd) : m_command(cmd), m_nodeFirstChild(nullptr), m_nodeNextSibling(nullptr) {}
    ~ASTNode() = default;

    Command m_command;
    ASTNode* m_nodeFirstChild = nullptr;
    ASTNode* m_nodeNextSibling = nullptr;
};