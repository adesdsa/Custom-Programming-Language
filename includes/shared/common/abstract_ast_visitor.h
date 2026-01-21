#pragma once

#include <unordered_map>
#include <functional>

#include "includes/frontend/parser/ast.h"
#include "includes/frontend/parser/command.h"

/// @brief Imploment abstract visitor design pattern for AST object.
/// @see Visitor deisgn pattern  ||
///                              \/ 
/// @link https://www.geeksforgeeks.org/system-design/visitor-design-pattern/
class IASTVisitor
{
public:
    virtual ~IASTVisitor() = default;

    // Prevent copying/moving - lambdas capture 'this' pointer
    IASTVisitor(const IASTVisitor&) = delete;
    IASTVisitor& operator=(const IASTVisitor&) = delete;
    IASTVisitor(IASTVisitor&&) = delete;
    IASTVisitor& operator=(IASTVisitor&&) = delete;

    /// @brief Call the right handler method based on the node command type:
    ///     `node->command->GetType()`
    /// @param node: Node to visit
    void Visit(ASTNode* node);

protected:
    /// @brief Default constructor for derived classes
    IASTVisitor() = default;

    /**
        ******************************************************************
        * Following methods are handler methods for each type.           *
        * Each child of this abstract class will imploment those methods *
        * based on his needs.                                            *
        ******************************************************************
    */

    virtual void VisitVarDeclaration(ASTNode* node) = 0;
    virtual void VisitExpression(ASTNode* node) = 0;
    virtual void VisitIdentifier(ASTNode* node) = 0;
    virtual void VisitIdentifierValue(ASTNode* node) = 0;
    virtual void VisitAssignment(ASTNode* node) = 0;
    virtual void VisitFuncDeclaration(ASTNode* node) = 0;
    virtual void VisitFuncDefinition(ASTNode* node) = 0;
    virtual void VisitFuncCall(ASTNode* node) = 0;
    virtual void VisitIf(ASTNode* node) = 0;
    virtual void VisitElif(ASTNode* node) = 0;
    virtual void VisitElse(ASTNode* node) = 0;
    virtual void VisitWhile(ASTNode* node) = 0;
    virtual void VisitFor(ASTNode* node) = 0;
    virtual void VisitReturn(ASTNode* node) = 0;
    virtual void VisitControlStatement(ASTNode* node) = 0;
    virtual void VisitBlockStart(ASTNode* node) = 0;
    virtual void VisitBlockEnd(ASTNode* node) = 0;
    virtual void VisitEndOfCommand(ASTNode* node) = 0;
    virtual void VisitSizeof(ASTNode* node) = 0;
    virtual void VisitVoidCommand(ASTNode* node) = 0;
    virtual void VisitPass(ASTNode* node) = 0;
    virtual void VisitEof(ASTNode* node) = 0;

    /// @brief Initialize dispatch map, each child should call it in constructor
    void InitializeHandlers();

private:
    /// key: CommandType, @see command.h 
    /// value: Matching handler, defined in dispatch map
    std::unordered_map<std::string_view, std::function<void(ASTNode*)>> m_handlers;
};
