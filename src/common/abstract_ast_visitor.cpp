#include "includes/common/abstract_ast_visitor.h"

void IASTVisitor::Visit(ASTNode* node)
{
    if (!node) return;

    auto commandHandler = this->m_handlers.find(node->command.GetType());

    if (commandHandler != m_handlers.end())
    {
        commandHandler->second(node);  // call the corresponding handler method
    }
}

void IASTVisitor::InitializeHandlers()
{
    this->m_handlers = {
        {CommandTypes::VAR_DECLARATION,          [this](ASTNode* n) { VisitVarDeclaration(n); }},
        /*
        {CommandTypes::IDENTIFIER,               [this](ASTNode* n) { VisitIdentifier(n); }},
        {CommandTypes::IDENTIFIER_VALUE,         [this](ASTNode* n) { VisitIdentifierValue(n); }},
        {CommandTypes::ASSIGNMENT,               [this](ASTNode* n) { VisitAssignment(n); }},
        {CommandTypes::EXPRESSION,               [this](ASTNode* n) { VisitExpression(n); }},
        {CommandTypes::FINISHED_FUNC_DECLARATION,[this](ASTNode* n) { VisitFuncDeclaration(n); }},
        {CommandTypes::FINISHED_FUNC_CALL,       [this](ASTNode* n) { VisitFuncCall(n); }},
        {CommandTypes::IF,                       [this](ASTNode* n) { VisitIf(n); }},
        {CommandTypes::ELIF,                     [this](ASTNode* n) { VisitElif(n); }},
        {CommandTypes::ELSE,                     [this](ASTNode* n) { VisitElse(n); }},
        {CommandTypes::WHILE,                    [this](ASTNode* n) { VisitWhile(n); }},
        {CommandTypes::FOR,                      [this](ASTNode* n) { VisitFor(n); }},
        {CommandTypes::RETURN,                   [this](ASTNode* n) { VisitReturn(n); }},
        {CommandTypes::CONTROL_STATEMENT,        [this](ASTNode* n) { VisitControlStatement(n); }},
        {CommandTypes::BLOCK_START,              [this](ASTNode* n) { VisitBlockStart(n); }},
        {CommandTypes::BLOCK_END,                [this](ASTNode* n) { VisitBlockEnd(n); }},
        {CommandTypes::END_OF_COMMAND,           [this](ASTNode* n) { VisitEndOfCommand(n); }},
        {CommandTypes::ARRAY_DECLARATION,        [this](ASTNode* n) { VisitArrayDeclaration(n); }},
        {CommandTypes::ARRAY_ACCESS,             [this](ASTNode* n) { VisitArrayAccess(n); }},
        {CommandTypes::ARRAY_VALUE,              [this](ASTNode* n) { VisitArrayValue(n); }},
        {CommandTypes::SIZEOF,                   [this](ASTNode* n) { VisitSizeof(n); }},
        {CommandTypes::VOID_COMMAND,             [this](ASTNode* n) { VisitVoidCommand(n); }},
        {CommandTypes::PASS,                     [this](ASTNode* n) { VisitPass(n); }},
        {CommandTypes::EOF_TOKEN,                [this](ASTNode* n) { VisitEof(n); }},
        */
    };
}
