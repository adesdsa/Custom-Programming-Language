#include "includes/shared/common/abstract_ast_visitor.h"

void IASTVisitor::Visit(ASTNode* node)
{
    if (!node) return;

    auto commandHandler = this->m_handlers.find(node->command.GetType());

    if (commandHandler != m_handlers.end())
    {
        try
        {
            commandHandler->second(node);  // call the corresponding handler method
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
    }
}

void IASTVisitor::InitializeHandlers()
{
    this->m_handlers.reserve(23);  // pre-allocate for efficiency
    
    this->m_handlers.emplace(CommandTypes::VAR_DECLARATION,          [this](ASTNode* n) { VisitVarDeclaration(n); });
    this->m_handlers.emplace(CommandTypes::EXPRESSION,               [this](ASTNode* n) { VisitExpression(n); });
    this->m_handlers.emplace(CommandTypes::EXPRESSION_VALUE,         [this](ASTNode* n) { VisitExpression(n); });
    this->m_handlers.emplace(CommandTypes::EXPRESSION_OPERATOR,      [this](ASTNode* n) { VisitExpression(n); });
    this->m_handlers.emplace(CommandTypes::IDENTIFIER,               [this](ASTNode* n) { VisitIdentifier(n); });
    this->m_handlers.emplace(CommandTypes::ARRAY_ACCESS,             [this](ASTNode* n) { VisitArrayAccess(n); });
    this->m_handlers.emplace(CommandTypes::ARRAY_SLICE,              [this](ASTNode* n) { VisitArrayAccess(n); });
    this->m_handlers.emplace(CommandTypes::IDENTIFIER_VALUE,         [this](ASTNode* n) { VisitIdentifierValue(n); });
    this->m_handlers.emplace(CommandTypes::ASSIGNMENT,               [this](ASTNode* n) { VisitAssignment(n); });
    this->m_handlers.emplace(CommandTypes::FINISHED_FUNC_DECLARATION,[this](ASTNode* n) { VisitFuncDeclaration(n); });
    this->m_handlers.emplace(CommandTypes::FINISHED_FUNC_DEFINITION, [this](ASTNode* n) { VisitFuncDefinition(n); });
    this->m_handlers.emplace(CommandTypes::FINISHED_FUNC_CALL,       [this](ASTNode* n) { VisitFuncCall(n); });
    this->m_handlers.emplace(CommandTypes::IF,                       [this](ASTNode* n) { VisitIf(n); });
    this->m_handlers.emplace(CommandTypes::ELIF,                     [this](ASTNode* n) { VisitElif(n); });
    this->m_handlers.emplace(CommandTypes::ELSE,                     [this](ASTNode* n) { VisitElse(n); });
    this->m_handlers.emplace(CommandTypes::WHILE,                    [this](ASTNode* n) { VisitWhile(n); });
    this->m_handlers.emplace(CommandTypes::FOR,                      [this](ASTNode* n) { VisitFor(n); });
    this->m_handlers.emplace(CommandTypes::RETURN,                   [this](ASTNode* n) { VisitReturn(n); });
    this->m_handlers.emplace(CommandTypes::CONTROL_STATEMENT,        [this](ASTNode* n) { VisitControlStatement(n); });
    this->m_handlers.emplace(CommandTypes::BLOCK_START,              [this](ASTNode* n) { VisitBlockStart(n); });
    this->m_handlers.emplace(CommandTypes::BLOCK_END,                [this](ASTNode* n) { VisitBlockEnd(n); });
    this->m_handlers.emplace(CommandTypes::END_OF_COMMAND,           [this](ASTNode* n) { VisitEndOfCommand(n); });
    this->m_handlers.emplace(CommandTypes::SIZEOF,                   [this](ASTNode* n) { VisitSizeof(n); });
    this->m_handlers.emplace(CommandTypes::VOID_COMMAND,             [this](ASTNode* n) { VisitVoidCommand(n); });
    this->m_handlers.emplace(CommandTypes::IMPORT_STATEMENT,         [this](ASTNode* n) { VisitImportStatement(n); });
    this->m_handlers.emplace(CommandTypes::PASS,                     [this](ASTNode* n) { VisitPass(n); });
    this->m_handlers.emplace(CommandTypes::EOF_TOKEN,                [this](ASTNode* n) { VisitEof(n); });
}
