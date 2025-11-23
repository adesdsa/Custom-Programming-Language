#pragma once

#include "ast_node.h"
// #include <cstddef>

// The AST itself. Hold all Nodes of the source file.
class AST
{
public:
    AST() = default;
    ~AST();

    ASTNode* AllocateNode(const Token& token);

    // Free all allocated nodes in the ast and remove them from the nodes vector
    void Clear();

private:
    std::vector<class ASTNode*> m_nodes;
};
 