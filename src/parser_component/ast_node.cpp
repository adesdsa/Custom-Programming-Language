#include "parser_component/ast_node.h"

ASTNode::ASTNode(const Command& command) : 
    m_command(command), 
    m_nodeFirstChild(nullptr), 
    m_nodeNextSibling(nullptr) {}