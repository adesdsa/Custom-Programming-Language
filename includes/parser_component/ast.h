#pragma once

#include "ast_node.h"
// #include <cstddef>

// The AST itself. Hold all Nodes of the source file.
class AST
{
public:
    ASTArena() = default;

    ~ASTArena() { Clear(); }

    template <typename... Args>
    class ASTNode* AllocateNode(Args&&... args)
    {
        nodes.emplace_back(new ASTNode(std::forward<Args>(args)...));
        return nodes.back();
    }

    void Clear()
    {
        for (auto* n : nodes)
            delete n;
        nodes.clear();
    }

private:
    std::vector<class ASTNode*> nodes;
};
