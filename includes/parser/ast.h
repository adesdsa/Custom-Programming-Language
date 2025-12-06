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

    ASTNode* GetRoot() const noexcept;

    ASTNode* AllocateNode(const Command& command);

    // Build a command subtree from given full command onto AST
    // @param fullCommand: CommandLine object that represents vector of commands
    // the vector is sorted from first token in the command to the last one.
    void StoreFullCommand(const CommandLine& fullCommand);

    // debug purpose only - remove comment when needed
    void PrintAST() const;

private:
    std::vector<std::unique_ptr<ASTNode>> m_nodes;  // memory ownership
    ASTNode* m_root = nullptr;  // AST entry point
    ASTNode* m_leaf = nullptr;  // AST end point
    const EntityLogger& m_entityLogger;

    // Recursively stores all parameter layers of `command` as 
    // children in the AST subtree rooted at `parent`.
    // @param parent: The AST subtree node to attach the parameters to.
    // @param command: The command whose parameters should be stored.
    void StoreParams(ASTNode* parent, const Command& command);

    // debug purpose only - remove comment when needed
    void PrintASTNode(const ASTNode* node, int indent) const;
};
