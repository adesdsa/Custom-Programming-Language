#pragma once

#include <memory>

#include "ast_node.h"

// The AST itself. Holds all Nodes of the source file.
class AST
{
public:
    AST() = default;
    ~AST();

    void Clear();

    ASTNode* AllocateNode(const Command& command);

    // Adds a command subtree from given full command
    // @param ast: The ast that holds this node.
    // @param fullCommand: CommandLine object that represents vector of commands
    // the vector is sorted from first token in the command to the last one.
    void StoreFullCommand(const CommandLine& fullCommand);
    void PrintAST() const;
    void PrintASTNode(const ASTNode* node, int indent = 0);
private:
    std::vector<std::unique_ptr<ASTNode>> m_nodes;
    
    void PrintASTNode(const ASTNode* node, int indent) const;
};
