#include "includes/parser/ast_extractor.h"

std::optional<std::string> ASTExtractor::ExtractVarName(ASTNode* varDecNode)
{    
    for (const Token& token : varDecNode->command.GetTokens())
    {
        if (token.GetType() == TokenTypes::IDENTIFIER)
        {
            return token.GetValue();
        }
    }

    return std::nullopt;
}

std::optional<Type> ASTExtractor::ExtractVarType(ASTNode* varDecNode)
{
    for (const Token& token : varDecNode->command.GetTokens())
    {
        std::optional<Type> type = StringToType(token.GetValue());
        if (type.has_value())
        {
            return type;
        }
    }

    return std::nullopt;
}

Scope ASTExtractor::ExtractVarScope(ASTNode* varDecNode)
{
    for (const Token& token : varDecNode->command.GetTokens())
    {
        std::string_view val = token.GetValue();
        if (val == "var") return Scope::VAR;
        if (val == "let") return Scope::LET;
        if (val == "global") return Scope::GLOBAL;
    }
    
    return Scope::LET;  // Default scope
}

AccessRights ASTExtractor::ExtractVarAccessRight(ASTNode* varDecNode)
{
    for (const Token& token : varDecNode->command.GetTokens())
    {
        std::string_view val = token.GetValue();
        if (val == "const") return AccessRights::CONST_ACCESS;
        if (val == "readonly") return AccessRights::READ_ONLY_ACCESS;
    }
    
    return AccessRights::DEFAULT_ACCESS;
}

std::optional<Type> ASTExtractor::StringToType(std::string_view type)
{
    // TODO: Add unsigned support
    if (type == "void") return Type::VOID_TYPE;
    if (type == "bool") return Type::BOOL_TYPE;
    if (type == "int") return Type::INT_TYPE;
    if (type == "double") return Type::DOUBLE_TYPE;

    return std::nullopt;  // Unknown/unsupported type
}
