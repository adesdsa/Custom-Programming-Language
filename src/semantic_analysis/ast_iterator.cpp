#include "includes/semantic_analysis/ast_iterator.h"

ASTIterator::ASTIterator(const AST& ast) :
    m_cursor(ast.GetRoot()),
    m_parentStack()  // At the start no parents
{}

const ASTNode* ASTIterator::GetCurrent() const noexcept
{
    return this->m_cursor;
}

bool ASTIterator::HasNext() const noexcept
{
    return this->m_cursor != nullptr;
}

bool ASTIterator::Next()
{
    if (!this->m_cursor) return false;

    // IF: There is a first child → Go to first child
    if (this->m_cursor->nodeFirstChild)
    {
        this->m_parentStack.push_back(this->m_cursor);  // save current node as parent
        this->m_cursor = this->m_cursor->nodeFirstChild;
        return true;
    }

    // ELSE IF: There is a next sibling child → Go to next sibling
    else if (this->m_cursor->nodeNextSibling)
    {
        this->m_cursor = this->m_cursor->nodeNextSibling;
        return true;
    }
    
    // ELSE IF: There is a parent node: → Climb up until a parent has a next sibling
    while (!this->m_parentStack.empty())
    {
        ASTNode* parent = this->m_parentStack.back();
        this->m_parentStack.pop_back();

        if (parent->nodeNextSibling)
        {
            this->m_cursor = parent->nodeNextSibling;
            return true;
        }
    }

    // ELSE: → Finished iterating over the AST
    this->m_cursor = nullptr;
    return false;
}
