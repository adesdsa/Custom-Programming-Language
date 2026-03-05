#include "includes/frontend/parser/ast_extractor.h"

const std::string& ASTExtractor::ExtractVarName(ASTNode* varDecNode)
{    
    for (const Token& token : varDecNode->command.GetTokens())
    {
        if (token.GetType() == TokenTypes::IDENTIFIER)
        {
            return token.GetValue();
        }
    }

    throw std::exception();  // shouldn't get here after syntax checking
}

Type ASTExtractor::ExtractVarType(ASTNode* varDecNode)
{
    for (const Token& token : varDecNode->command.GetTokens())
    {
        std::optional<Type> type = StringToType(token.GetValue());
        if (type.has_value())
        {
            return type.value();
        }
    }

    throw std::exception();  // shouldn't get here after syntax checking
}

Scope ASTExtractor::ExtractVarScope(ASTNode* varDecNode)
{
    for (const Token& token : varDecNode->command.GetTokens())
    {
        std::string_view val = token.GetValue();
        if (val == "var") return Scope::VAR;
        if (val == "let") return Scope::LET;
        if (val == "global") return Scope::GLOBAL;
        if (val == "export") return Scope::EXPORT;
    }
    
    return Scope::LET;  // Default scope
}

Type ASTExtractor::ExtractFuncReturnType(ASTNode* funcDecNode)
{
    if (!funcDecNode) return Type::VOID_TYPE;
    
    // return type is the last type keyword in the function declaration tokens
    // (after the -> arrow). Children contain parameters, not return type.
    const auto& tokens = funcDecNode->command.GetTokens();

    for (auto token = tokens.rbegin(); token != tokens.rend(); token++)
    {
        std::optional<Type> type = StringToType(token->GetValue());
        if (type.has_value())
        {
            return type.value();
        }
    }
    
    return Type::VOID_TYPE;
}

const std::vector<VarMetaData> ASTExtractor::ExtractFuncParams(
    ASTNode* funcDecNode, 
    const VarFactory& varFactory
)
{
    std::vector<VarMetaData> params;
    
    if (!funcDecNode) return params;
    
    for (
        ASTNode* child = funcDecNode->nodeFirstChild; 
        child != nullptr; 
        child = child->nodeNextSibling
    )
    {
        // process VAR_DECLARATION children (these are parameters)
        if (child->command.GetType() == CommandTypes::VAR_DECLARATION)
        {
            const std::string& paramName = ExtractVarName(child);
            Type paramType = ExtractVarType(child);
            AccessRights paramAccess = ExtractVarAccessRight(child);
            
            Scope paramScope = ExtractVarScope(child);
            params.push_back(
                varFactory.CreateVarMetadata(
                    paramName,
                    paramType,
                    paramAccess,
                    paramScope
                )
            );
        }
    }

    return params;
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
