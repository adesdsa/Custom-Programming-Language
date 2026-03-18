#include "includes/frontend/parser/ast_extractor.h"
#include "includes/frontend/parser/command.h"
#include "includes/frontend/tokenizer/token.h"

#include <cstdlib>
#include <cctype>

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

std::optional<size_t> ASTExtractor::ExtractArraySize(ASTNode* arrayDeclNode)
{
    if (!arrayDeclNode || arrayDeclNode->command.GetType() != CommandTypes::ARRAY_DECLARATION)
    {
        return std::nullopt;
    }

    const auto& params = arrayDeclNode->command.GetParams();
    if (params.empty()) 
    {
        return std::nullopt;
    }

    const Command& sizeParam = params[0];
    const auto& tokens = sizeParam.GetTokens();

    // Only accept a single integer literal (e.g. "10")
    for (const Token& token : tokens)
    {
        if (token.GetType() != TokenTypes::VALUE)
        {
            return std::nullopt;  // Non-value token (e.g. operator) -> not a simple literal
        }
    }
    if (tokens.size() != 1)
    {
        return std::nullopt;  // "2+3" or "3, 2" would have multiple tokens
    }

    const std::string& val = tokens[0].GetValue();
    if (val.empty()) return std::nullopt;
    
    const char* p = val.c_str();
    for (; *p && std::isdigit(static_cast<unsigned char>(*p)); p++) {}
    if (*p != '\0')
    {
        return std::nullopt;  // Non-integer (e.g. "10.5")
    }
    unsigned long long n = std::strtoull(val.c_str(), nullptr, 10);
    if (n <= SIZE_MAX)
    {
        return static_cast<size_t>(n);
    }
    return std::nullopt;
}

std::optional<size_t> ASTExtractor::ExtractArraySizeFromInitializer(ASTNode* arrayDeclNode)
{
    if (!arrayDeclNode) return std::nullopt;

    ASTNode* assignNode = arrayDeclNode->nodeNextSibling;
    if (!assignNode) return std::nullopt;

    if (assignNode->command.GetType() != CommandTypes::ASSIGNMENT) return std::nullopt;

    ASTNode* arrayValueNode = assignNode->nodeNextSibling;
    if (!arrayValueNode) return std::nullopt;
    if (arrayValueNode->command.GetType() != CommandTypes::ARRAY_VALUE)
    {
        return std::nullopt;
    }

    const auto& params = arrayValueNode->command.GetParams();
    size_t count = params.size();
    if (count == 0 || count > SIZE_MAX)
    {
        return std::nullopt;
    }
    return count;
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
                    paramScope,
                    std::nullopt
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
    if (type == "char") return Type::CHAR_TYPE;
    if (type == "string") return Type::STRING_TYPE;

    return std::nullopt;  // Unknown/unsupported type
}
