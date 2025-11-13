#include "../../includes/parser_component/ast.h"

ASTNode::ASTNode(const Token &token) : 
    m_token(token),
    m_nodeFirstChild(nullptr),
    m_nodeNextSibling(nullptr) {}