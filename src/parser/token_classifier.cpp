#include "includes/parser/token_classifier.h"

bool TokenClassifier::IsValue(const Token& token, const std::string& sCompare)
{
    return token.GetValue() == sCompare;
}

bool TokenClassifier::IsType(const Token &token, std::string_view sCompare)
{
    return token.GetType() == sCompare;
}

bool TokenClassifier::IsAccessRights(const Token& token)
{
    return IsValue(token, "const") || IsValue(token, "readonly");
}

bool TokenClassifier::IsScope(const Token& token)
{
    return (
            IsValue(token, "global") ||
            IsValue(token, "var")    || 
            IsValue(token, "let")
        );
}

bool TokenClassifier::IsExpressionStart(const Token &token)
{
    return (
            IsType(token, TokenTypes::IDENTIFIER) ||
            IsType(token, TokenTypes::VALUE)      ||
            IsValue(token, "-")                   ||
            IsValue(token, "not")                 ||
            IsValue(token, "*")                   ||
            IsValue(token, "&")                   ||
            IsValue(token, "(")
    );
}

bool TokenClassifier::IsValueToken(const Token &token)
{
    return IsType(token, TokenTypes::IDENTIFIER) || IsType(token, TokenTypes::VALUE);
}

bool TokenClassifier::IsOperator(const Token &token)
{
    if (IsType(token, TokenTypes::OPERATOR))
    {
        return !IsValue(token, "=") && !IsValue(token, "&");
    }

    return (
            IsValue(token, "and") ||
            IsValue(token, "or")  ||
            IsValue(token, "xor") ||
            IsValue(token, "is") 
        );
}

bool TokenClassifier::IsType(const Token &token)
{
    return (
            IsValue(token, "int")    ||
            IsValue(token, "bool")   ||
            IsValue(token, "double") ||
            IsValue(token, "void") 
        );
}

bool TokenClassifier::IsControlStatement(const Token &token)
{
    return IsValue(token, "break") || IsValue(token, "continue");
}
