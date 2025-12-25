#pragma once

#include "ast.h"

/// @brief On-demand + depth-first iterator for given ast.
class ASTIterator
{
    ASTIterator(const AST& ast);
    ~ASTIterator() = default;

    /// @brief Look at the current node (holded by iterator cursor)
    /// @return const ASTNode* : current AST node from cursor
    const ASTNode* GetCurrent() const noexcept;

    /// @brief Advance to next node in following order:
        /// IF: There is a first child → Go to first child
        /// ELSE IF: There is a next sibling child → Go to next sibling
        /// ELSE IF: There is a parent node: → Climb up until a parent has a next sibling
        /// ELSE: → Finished iterating over the AST
    /// @return bool : true -> Sucessfully advanced to next node. false -> Otherwise.
    bool Next();

private:
    ASTNode* m_cursor;  // points to the currect ASTNode
    // helper - each time get into a children node hold it parent in the stack
    std::vector<ASTNode*> m_parentStack;
};
