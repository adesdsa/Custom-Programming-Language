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
    AST(const EntityLogger& entityLogger);
    ~AST();

    void Clear();

    ASTNode* AllocateNode(const Command& command);

    // Build a command subtree from given full command onto AST
    // @param fullCommand: CommandLine object that represents vector of commands
    // the vector is sorted from first token in the command to the last one.
    void StoreFullCommand(const CommandLine& fullCommand);

    void PrintAST() const;

private:
    std::vector<std::unique_ptr<ASTNode>> m_nodes;  // memory ownership
    ASTNode* m_root = nullptr;  // AST entry point
    ASTNode* m_leaf = nullptr;  // AST end point
    const EntityLogger& m_entityLogger;

    // Recursively add layer of paramters of command to the given subtree, parent.
    // @param parent: Subtree of AST to build the params on top of.
    // @param command: The command of which the params belongs to.
    void StoreParams(ASTNode* parent, const Command& command);
    void PrintASTNode(const ASTNode* node, int indent) const;
};
