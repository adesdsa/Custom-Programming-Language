#include "parser/ast.h"

AST::~AST() 
{
    this->Clear();
}

void AST::Clear()
{
    m_nodes.clear();  // deletes all nodes
}

ASTNode* AST::AllocateNode(const Command& command)
{
    this->m_nodes.push_back(std::make_unique<ASTNode>(command));
    return this->m_nodes.back().get();
}

void AST::StoreFullCommand(const CommandLine& fullCommand)
{
    if (!fullCommand.HasCommands()) return;
    
    ASTNode* prevSibling = nullptr;
    ASTNode* root = nullptr;

    for (const Command& command : fullCommand.GetLine())
    {
        ASTNode* node = AllocateNode(command);

        if (!root)
        {
            root = node;
        }

        if (prevSibling)
        {
            prevSibling->nodeNextSibling = node;
        }

        // add command parameters as children first
        if (command.HasParams())
        {
            ASTNode* paramPrev = nullptr;
            for (const Command& param : command.GetParams())
            {
                ASTNode* paramNode = AllocateNode(param);

                if (!paramPrev)
                {
                    node->nodeFirstChild = paramNode;
                }
                else
                {
                    paramPrev->nodeNextSibling = paramNode;
                }

                paramPrev = paramNode;
            }
        }

        prevSibling = node;
    }
}

void AST::PrintASTNode(const ASTNode* node, int indent) const 
{
    if (!node)
        return;

    // Print indentation
    for (int i = 0; i < indent; ++i)
        std::cout << "  ";

    // Print node info (type + optional first token value)
    std::cout << node->command.GetType();

    const auto& tokens = node->command.GetTokens();
    if (!tokens.empty())
        std::cout << " (" << tokens.front().GetValue() << ")";

    std::cout << std::endl;

    // Recursively print children first (parameters)
    PrintASTNode(node->nodeFirstChild, indent + 1);

    // Then print next sibling
    PrintASTNode(node->nodeNextSibling, indent);
}

// Print the entire AST starting from first stored node
void AST::PrintAST() const
{
    if (m_nodes.empty())
    {
        std::cout << "<empty AST>" << std::endl;
        return;
    }

    for (const auto& nodePtr : m_nodes)
    {
        PrintASTNode(nodePtr.get(),0);
    }
}
