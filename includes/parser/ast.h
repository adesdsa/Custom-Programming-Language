#pragma once

#include <memory>

#include "commandLine.h"

// Left-child, right-sibling AST node
// Each node contains full command (function, variable decleraion  etc...)
struct ASTNode
{
    Command command;
    ASTNode* nodeFirstChild;
    ASTNode* nodeNextSibling;

    ASTNode(const Command& command) : 
        command(command), 
        nodeFirstChild(nullptr), 
        nodeNextSibling(nullptr) {}
};

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

private:
    std::vector<std::unique_ptr<ASTNode>> m_nodes;

    void PrintASTNode(const ASTNode* node, int indent) const;
};
