#include "includes/parser/ast.h"

AST::AST(const EntityLogger& entityLogger) : 
    m_nodes(),
    m_root(nullptr),
    m_leaf(nullptr),
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
    this->m_nodes.clear();  // deletes all nodes
    this->m_root = nullptr;
    this->m_leaf = nullptr;
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
    
    ASTNode* subtreeRoot = nullptr;
    ASTNode* prevSibling = nullptr;

    for (const Command& command : fullCommand.GetLine())
    {
        ASTNode* node = AllocateNode(command);

        if (!subtreeRoot)
        {
            subtreeRoot = node;
        }

        if (prevSibling)
        {
            prevSibling->nodeNextSibling = node;
        }

        if(command.HasParams())
        {
            this->StoreParams(node, command);
        }

        prevSibling = node;
    }

    if (!this->m_root)
    {
        this->m_root = subtreeRoot;
        this->m_leaf = prevSibling;
    }
    else
    {
        this->m_leaf->nodeNextSibling = subtreeRoot;
        this->m_leaf = prevSibling;
    }
}

void AST::StoreParams(ASTNode* parent, const Command& command)
{
    if (!parent || !command.HasParams()) return;

    ASTNode* prevSibling = nullptr;

    for (const Command& param : command.GetParams())
    {
        ASTNode* paramNode = AllocateNode(param);

        if (!prevSibling)
        {
            parent->nodeFirstChild = paramNode;
        }
        else
        {
            prevSibling->nodeNextSibling = paramNode;
        }

        prevSibling = paramNode;

        this->StoreParams(paramNode, param);
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
