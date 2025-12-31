#pragma once

#include <unordered_map>
#include <functional>

#include "includes/parser/ast.h"
#include "includes/parser/command.h"

/// @brief Imploment abstract visitor design pattern for AST object.
/// @see Visitor deisgn pattern  ||
///                              \/ 
/// @link https://www.geeksforgeeks.org/system-design/visitor-design-pattern/
class IASTVisitor
{
public:
    virtual ~IASTVisitor() = default;

    /// @brief Call the right handler method based on the node command type:
    ///     `node->command->GetType()`
    /// @param node: Node to visit
    void Visit(ASTNode* node);

protected:
    /**
        ******************************************************************
        * Following methods are handler methods for each type.           *
        * Each child of this abstract class will imploment those methods *
        * based on his needs.                                            *
        ******************************************************************
    */

    virtual void VisitVarDeclaration(ASTNode* node) = 0;
    virtual void VisitExpression(ASTNode* node) = 0;
    // virtual void VisitIdentifier(ASTNode* node) = 0;
    // ... etc

    /// @brief Initialize dispatch map, each child should call it in constructor
    void InitializeHandlers();

private:
    /// key: CommandType, @see command.h 
    /// value: Matching handler, defined in dispatch map
    std::unordered_map<std::string_view, std::function<void(ASTNode*)>> m_handlers;
};
