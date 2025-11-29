#include "parser/ast.h"

AST::AST(const EntityLogger& entityLogger) : 
    m_nodes(),
    m_root(nullptr),
    m_entityLogger(entityLogger) 
{
    this->m_entityLogger.Info("Finished initializing parser's AST successfully");
}

AST::~AST() 
{
    this->Clear();
}

void AST::Clear()
{
    m_nodes.clear();  // deletes all nodes
    m_root = nullptr;
}

ASTNode* AST::AllocateNode(const Command& command)
{
    this->m_nodes.push_back(std::make_unique<ASTNode>(command));
    return this->m_nodes.back().get();
}

void AST::StoreFullCommand(const CommandLine& fullCommand)
{
    if (!fullCommand.HasCommands()) return;

    this->m_entityLogger.Info("Building new full-command onto AST");
    
    ASTNode* prevSibling = nullptr;
    ASTNode* localRoot = nullptr;

    for (const Command& command : fullCommand.GetLine())
    {
        ASTNode* node = AllocateNode(command);

        if (!localRoot)
        {
            localRoot = node;
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

    if (!this->m_root)
    {
        this->m_root = localRoot;
    }
    else
    {
        ASTNode* last = this->m_root;

        while(last->nodeNextSibling)
        {
            last = last->nodeNextSibling;
        }

        last->nodeNextSibling = localRoot;
    }
}

void AST::PrintASTNode(const ASTNode* node, int indent) const 
{
    if (!node) return;

    for (int i = 0; i < indent; ++i)
        std::cout << "  ";

    std::cout << node->command.GetType();

    const auto& tokens = node->command.GetTokens();
    if (!tokens.empty())
        std::cout << " (" << tokens.front().GetValue() << ")";

    std::cout << "\n";

    PrintASTNode(node->nodeFirstChild, indent + 1);
    PrintASTNode(node->nodeNextSibling, indent);
}

// Print the entire AST starting from first stored node
void AST::PrintAST() const
{
    if (!this->m_root)
    {
        std::cout << "<empty AST>\n";
        return;
    }
    this->PrintASTNode(this->m_root, 0);
}
